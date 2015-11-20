#include "misc.h"
#include "generic.h"

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

void usage()
{
    info("Usage:\n");
    info("    flash --firehose prog_emmc_firehose_8909_ddr.mbn --rawprogram rawprogram0.xml --patch patch0.xml --imagedir /tmp/iamges --reboot --format");
    info("");
    info("    --reboot is optional");
    info("    --format is optional");
    info("    --patch is optional");
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
        
         "some images are sparse=\"true\" in rawprogram0.xml, you need convert them to rawimg(non-sparse) with sim2img tool before you run the command\n\n" 
        
        );

    info("\n");
    exit(EXIT_FAILURE);
}
