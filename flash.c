#define _GNU_SOURCE
#include "sahara.h"
#include "firehose.h"
#include "device.h"
#include "defs.h"
#include "firehose_flash_image.h"

size_t get_file_size(int fd, size_t *size)
{
    struct stat buf;
    fstat(fd, &buf);
    *size = buf.st_size;
    return buf.st_size;
}

void print_stage_info(char *s)
{
    printf("----------------------------------------------\n");
    printf("%s\n", s); 
    printf("----------------------------------------------\n");
}

static void usage()
{
    info("Usage:\n");
    info("    flash --firehose prog_emmc_firehose_8909_ddr.mbn --rawprogram rawprogram0.xml --patch patch0.xml --imagedir /tmp/iamges --reboot --format --device SERIAL_NUMBER --simlock simlock");
    info("");
    info("    --reboot is optional");
    info("    --format is optional");
    info("    --patch is optional");
    info("    --imagedir specify the directory you store the images, this command only flash the images in the imagedir and also in rawprogram0.xml. be careful with the filenames, they shall be the same");
    info("    --list(-l)    list all devices of qdl mode, if you run command with --list, any other option will be ignored, run flash --list the list all the available devices with qdl mode");
    info("    --device(-s) <SERIAL_NUMBER>   flash the specified device with serial number, if there's only one device, this is not needed");
    info("    --simlock simlock or --simlock program");
    info("eg:");
    info("if you want to flash boot.img and system.img, you need to download them and put them into a direcotry,\n"
         "let's assume the direcotry is named vAJ3, any name you like, and download the rawprogram0.xml.\n"
         "make sure the system.img shall be a raw ext3(ext4) filesystem. the prog_emmc_firehose_8909_ddr.mbn is from amss,\n"
         "it's the same as NPRG36.bin which is used to write the images to emmc disk.\n"
         "run command below to start:\n"

         "\n\nflash --firehose PATH_TO_prog_emmc_firehose_8909_ddr.mbn --rawprogram PATH_TO_rawprogram0.xml --imagedir PATH_TO_vAJ3 --reboot\n\n\n"

         "the --reboot ask the device to reboot after flashing. the device would poweroff by default if you didn't add this.\n\n"

         "the --format will ask the device to format the partitions described as <zeroout> lines in rawprogram0.xml. if you don't know what you are doing, please don't add this.\n\n"

         "the --patch patch0.xml, will patch the gpt (partition table), you shall only need to add this when you flash gpt_main0.bin & gpt_backup0.bin.\n\n"

         "the --simlock specifies the tag to flash simlock partition, --simlock simlock or --simlock program \n\n"

         "some images are sparse=\"true\" in rawprogram0.xml, you need convert them to rawimg(non-sparse) with sim2img tool before you run the command\n\n"

        );

    info("\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    char *option = NULL;
    static struct option long_options[] = {
        {"firehose",   required_argument,  0,  0 },
        {"patch",    required_argument,  0,  0 },
        {"rawprogram",   required_argument,  0,  0 },
        {"imagedir",   required_argument,  0,  0 },
        {"simlock",   required_argument,  0,  0 },
        {"reboot",   no_argument,  0,  1},
        {"format",   no_argument,  0,  'f' },
        {"device",   required_argument,  0,  's' },
        {"list",   no_argument,  0,  'l' },
        {0,         0,       0,      0 }
    };

    char patch[128] = {0};
    char rawprogram[128] = {0};
    char imagedir[128] = {0};
    char firehose[128] = {0};
    char serial[256] = {0};
    char simlock[256] = {0};
    bool format_flag = False;
    bool reboot_flag = False;

    int c;
    int option_index = 0;
    while((c = getopt_long(argc, argv, "hfs:l", long_options, &option_index)) != -1){
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
                if(!strcmp(option, "simlock")){
                    strcpy(simlock, optarg);
                }
                break;
            case 'f':
                format_flag = True;
                break;
            case 1:
                reboot_flag = True;
                break;
            case 's':
                strcpy(serial, optarg);
                break;
            case 'l':
                print_qdl_devices();
                return;
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
    //print_stage_info("downloadling firehose");
    int status;
    status = qdl_usb_init(serial);
    if (status < 0){
        if(status == -2){
            print_qdl_devices();
            return -1;
        }else{
            xerror("qdl_usb_init error for %s\n", serial);
        }
    }
    print_stage_info("downloadling firehose");
    status = dowload_firehose_image(firehose);
    if (status < 0){
        xerror("dowload_firehose_image failed");
    }
    info("start running firehose\n");
    qdl_usb_close();
    sleep(3);

    /*
        sahara starts
    */
    qdl_usb_init(serial);
    response_t resp;

    resp = firehose_emmc_info();
    if (resp != ACK)
        xerror("firehose_emmc_info error");

    resp = process_firehose_configure();
    if (resp != ACK)
        xerror("process_firehose_configure error\n");

    size_t len;
    FILE *fp;
    char *line= NULL;
    /*
        formatting
    */
    if(format_flag){
        print_stage_info("formatting");
        fp = fopen(rawprogram, "r");
        if (!fp)
            exit(EXIT_FAILURE);

        while(getline(&line, &len, fp) != -1){
            if (!strcasestr(line, "<zeroout")){
                free(line);
                line = NULL;
                continue;
            }

            resp = process_firehose_erase_xml(line, strlen(line));
            free(line);
            line = NULL;
        }
        fclose(fp);
    };

#if 1
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
        size_t r = 0;
        size_t offset = 0;
        char path[256] = {0};
        xml_reader_t reader;
        firehose_program_t program;
        if (!strcasestr(line, "<program")){
            free(line);
            line = NULL;
            continue;
        }

        xmlInitReader(&reader, line, strlen(line)); 
        init_firehose_program_from_xml_reader(&reader, &program);

        if (!program.filename[0]){ //if filename's blank, continue
            free(line);
            line = NULL;
            continue;
        }

        strncpy(path, imagedir, strlen(imagedir));
        strncat(path, "/", 1);
        strncat(path, program.filename, strlen(program.filename));

        int fd = open(path, O_RDONLY);
        if(fd<0){   // there's no need to program this partition
            free(line);
            line = NULL;
            continue;
        }
        info("programming  %s", program.filename);
        offset = program.file_sector_offset * program.sector_size;
        lseek(fd, offset, SEEK_SET);

        if (!strcasecmp(program.label, "simlock") && !strcasecmp(simlock, "simlock")){
            //simlock, a little dirty to catch
            printf("via <simlock>\n");
            size_t len = 0;
            firehose_simlock_t slk;
            get_file_size(fd, &len);
            slk.len = len; //have to set it here
            xml_reader_t simlock_reader;
            xmlInitReader(&simlock_reader, line, strlen(line));
            init_firehose_simlock_from_xml_reader(&simlock_reader, &slk);
            resp = process_simlock_file(fd, slk);
        } else {
            //program
            if (program.sparse){
                resp = process_sparse_file(fd, program);
                if (resp == ACK)
                    info("%s succeed", program.filename);
            }else{
                resp = process_general_file(fd, program);
                if (resp == ACK)
                    info("%s succeed", program.filename);
            }
        }
        free(line);
        line = NULL;
        close(fd);
    }
    fclose(fp);
#endif
    /*
        processing patch0.xml
    */
    if (patch[0]){
        print_stage_info("patching");
        fp = fopen(patch, "r");
        if (!fp){
            perror(patch);
            exit(EXIT_FAILURE);
        }

        while(getline(&line, &len, fp) != -1){
            if(!strcasestr(line, "<patch")){ //this shall be more common
                free(line);
                line = NULL;
                continue;
            }

            if(!strcasestr(line, "filename=\"DISK\"")){
                free(line);
                line = NULL;
                continue;
            }

            resp = process_firehose_patch_xml(line, strlen(line));
            if (resp != ACK){
                xerror("process_firehose_patch_xml error");
            }
            free(line);
            line = NULL;
        };
        fclose(fp);
    }
    /*
        power reset
    */

    print_stage_info("power...");
    int loop = 5;
    while(loop--){
        resp = process_power_action(reboot_flag?"reset":"off");
        if (resp == ACK){
            info("%s succeeded", reboot_flag?"reboot":"poweroff");
            break;
        }
        if (resp == NAK){
            xerror("%s failed", reboot_flag?"reboot":"poweroff");
        }
    }
    qdl_usb_close();
    print_stage_info("all done:)");
    return 0;
}
