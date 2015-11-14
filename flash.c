#include "sahara.h"
#include "firehose.h"
#include "generic.h"
#include "misc.h"

int main(int argc, char **argv)
{
    /*
        firehose download
    */
    print_stage_info("downloadling firehose");
    qdl_usb_init();
    int ret = 0;
    ret = dowload_firehose_image("prog_emmc_firehose_8909_ddr.mbn");
    if (ret < 0){
        xerror("dowload_firehose_image failed");
    }
    info("start running firehose\n");
    qdl_usb_close();
    sleep(2);

    /*
        sahara starts
    */
    qdl_usb_init();

    char filename[128] = {0};
    size_t sector_size;
    size_t file_sector_offset;
    size_t offset;
    size_t file_size;
    size_t sector_numbers;
    size_t payload=0;
    char label[128] = {0};
    char start_sector[128];
    size_t len;
    FILE *fp;
    u8 physical_partition_number;
    response_t resp;

    while(!firehose_configure(&payload));
    if(!payload){
        xerror("firehose_configure error");
    }
    info("set maxpayload size==> %zu", payload);

    /*
        formatting patitions
    */
    print_stage_info("formatting");
    fp = fopen("erase.xml", "r");
    if (!fp)
        exit(EXIT_FAILURE);

    char *line= NULL;
    while(getline(&line, &len, fp) != -1){
        bzero(start_sector, sizeof(start_sector));
        bzero(label, sizeof(label));
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

    
    /*
        rawprogramming
    */
    print_stage_info("rawprogramming");

    fp = fopen("rawprogram0.xml", "r");
    if (!fp){
        perror("rawprogram0.xml");
        exit(EXIT_FAILURE);
    }

    while(getline(&line, &len, fp) != -1){
        bzero(start_sector, sizeof(start_sector));
        parse_program_xml(line, strlen(line), 
                      &file_sector_offset, &sector_size,
                      &sector_numbers, start_sector, filename);
        line = NULL;
        info("programming  %s", filename);
        offset = file_sector_offset * sector_size;
        int fd = open(filename, O_RDONLY);
        if(fd<0){
            perror(filename);
            continue;
        }
        lseek(fd, offset, SEEK_SET);
        get_file_size(fd, &file_size);
        sector_numbers = (file_size - offset + sector_size - 1)/sector_size;
        resp = transmit_file(fd, payload, sector_numbers, sector_size, start_sector, 0);
        if (resp != ACK){
            free(line);
            line = NULL;
            close(fd);
            qdl_usb_close();
            exit(-1);
        } 
        free(line);
        close(fd);
    };
    fclose(fp);

    /*
        processing patch0.xml
    */
    print_stage_info("patching");

    char patch[4096] = {0};
    fp = fopen("patch0.xml", "r");
    if (!fp){
        perror("patch0.xml");
        exit(EXIT_FAILURE);
    }

    while(getline(&line, &len, fp) != -1){
        generate_patch_xml(patch, line);
        send_patch_xml(patch, strlen(patch));
        resp = patch_response();
        if (resp != ACK)
            xerror("patch error");
        else
            info("patch ACK\n");
        free(line);
        line = NULL;
    };
    fclose(fp);

    /*
        power reset
    */
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

    qdl_usb_close();
    print_stage_info("succeeded :)");
    return 0;
}
