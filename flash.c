#define _GNU_SOURCE
#include <strings.h> //strcasecmp
#include <getopt.h>
#include "sahara.h"
#include "firehose.h"
#include "generic.h"
#include "misc.h"


int main(int argc, char **argv)
{
    char *option = NULL;
    static struct option long_options[] = {
        {"firehose",   required_argument,  0,  0 },
        {"patch",    required_argument,  0,  0 },
        {"rawprogram",   required_argument,  0,  0 },
        {"imagedir",   required_argument,  0,  0 },
        {"reboot",   no_argument,  0,  1},
        {"format",   no_argument,  0,  'f' },
        {0,         0,       0,      0 }
    };

    char patch[128] = {0};
    char rawprogram[128] = {0};
    char imagedir[128] = {0};
    char firehose[128] = {0};
    bool format_flag = False;
    bool reboot_flag = False;

    int c;
    int option_index = 0;
    while((c = getopt_long(argc, argv, "hf", long_options, &option_index)) != -1){
        switch (c){
            case 0:
                option = (char *)long_options[option_index].name;

                if(!strcmp(option, "patch")){
                    strcpy(patch, optarg);
                }
                if(!strcmp(option, "rawprogram")){
                    strcpy(rawprogram, optarg);
                }
                if(!strcmp(option, "imagedir")){
                    strcpy(imagedir, optarg);
                }
                if(!strcmp(option, "firehose")){
                    strcpy(firehose, optarg);
                }
                break;
            case 'f':
                format_flag = True;
                break;
            case 1:
                reboot_flag = True;
                break;
            case 'h':   //help
                usage();
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (!(rawprogram[0] && imagedir[0])){
        usage();
        exit(EXIT_FAILURE);
    }

    /*
        firehose download
    */
    int status;
    print_stage_info("downloadling firehose");
    qdl_usb_init();
    status = dowload_firehose_image(firehose);
    if (status < 0){
        xerror("dowload_firehose_image failed");
    }
    info("start running firehose\n");
    qdl_usb_close();
    sleep(2);

    /*
        sahara starts
    */
    qdl_usb_init();
    size_t payload=0;
    size_t len;
    FILE *fp;
    char *line= NULL;
    response_t resp;

    while(!firehose_configure(&payload));
    if(!payload){
        xerror("firehose_configure error");
    }
    info("set maxpayload size==> %zu", payload);

    /*
        formatting
    */
    if(format_flag){
        print_stage_info("formatting");
        fp = fopen(rawprogram, "r");
        if (!fp)
            exit(EXIT_FAILURE);

        while(getline(&line, &len, fp) != -1){
            char label[128] = {0};
            char start_sector[128] = {0};
            size_t sector_numbers = 0;
            char *zeroout = NULL;
            u8 physical_partition_number = 0;

            zeroout = strcasestr(line, "<zeroout"); //fixme
            if (!zeroout){
                free(line);
                line = NULL;
                continue;
            }

            parse_erase_xml(line, strlen(line), label,
                            start_sector, &sector_numbers,
                            &physical_partition_number);

            resp = partition_erase(start_sector,
                                   sector_numbers,
                                   physical_partition_number);

            if (resp == ACK)
                info("formatting %s succeeded ", label);
            else
                info("formating %s failed", label);
            free(line);
            line = NULL;
        }

        fclose(fp);
    };

    /*
        rawprogramming
    */
    print_stage_info("rawprogramming");

    fp = fopen(rawprogram, "r");
    if (!fp){
        perror("rawprogram0.xml");
        exit(EXIT_FAILURE);
    }

    while(getline(&line, &len, fp) != -1){
        char filename[128] = {0};
        char path[256] = {0};
        size_t sector_size;
        size_t file_sector_offset;
        size_t offset;
        size_t file_size;
        size_t sector_numbers;
        char label[128] = {0};
        char start_sector[128] = {0};
        u8 physical_partition_number;

        char *program = NULL;
        program = strcasestr(line, "<program"); //fixme
        if (!program){
            free(line);
            line = NULL;
            continue;
        }
        parse_program_xml(line, strlen(line),
                      &file_sector_offset, &sector_size,
                      &sector_numbers, start_sector, filename);

        if (!filename[0]){
            free(line);
            line = NULL;
            continue;
        }

        strncpy(path, imagedir, strlen(imagedir));
        strncat(path, "/", 1);
        strncat(path, filename, strlen(filename));

        int fd = open(path, O_RDONLY);
        if(fd<0){   // there's no need to program this partition
            free(line);
            line = NULL;
            continue;
        }
        info("programming  %s", filename);
        offset = file_sector_offset * sector_size;
        lseek(fd, offset, SEEK_SET);
        get_file_size(fd, &file_size);
        file_size -= offset;
        sector_numbers = (file_size - offset + sector_size - 1)/sector_size;
        resp = transmit_file(fd, sector_numbers, sector_size, start_sector, 0);
        if (resp != ACK){
            free(line);
            close(fd);
            qdl_usb_close();
            exit(-1);
        }
        free(line);
        line = NULL;
        close(fd);
    }
    fclose(fp);

    /*
        processing patch0.xml
    */
    if (patch[0]){
        print_stage_info("patching");
        char pat[4096] = {0};
        fp = fopen(patch, "r");
        if (!fp){
            perror(patch);
            exit(EXIT_FAILURE);
        }

        while(getline(&line, &len, fp) != -1){
            char *patch_tag = NULL;
            char what[2048] = {0};
            char filename[128] = {0};

            patch_tag = strcasestr(line, "<patch"); //fixme
            if (!patch_tag){
                free(line);
                line = NULL;
                continue;
            }

            parse_patch_xml(line, len, what, filename);
            if(strcasecmp(filename, "DISK")){
                free(line);
                line = NULL;
                continue;
            }

            generate_patch_xml(pat, line);
            send_patch_xml(pat, strlen(pat));
            resp = patch_response();
            if (resp != ACK)
                xerror("failure: %s", what);
            else
                info("success: %s", what);
            free(line);
            line = NULL;
        };
        fclose(fp);
    }

    /*
        power reset
    */

    if(reboot_flag){
        print_stage_info("power reset");
        int loop = 10;
        while(loop--){
            resp = power_action("reset");
            if (resp == ACK){
                info("power reset ... succeeded");
                break;
            }
            else{
                xerror("power reset ... failed");
            }
        }
    }

    qdl_usb_close();
    print_stage_info("all done:)");
    return 0;
}
