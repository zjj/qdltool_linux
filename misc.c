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
    info("    flash --firehose prog_emmc_firehose_8909_ddr.mbn --rawprogram rawprogram0.xml --patch patch0.xml --dir /tmp/iamges --reboot --format");
    info("");
    info("    --reboot is optional");
    info("    --format is optional");
    info("    --patch is optional");
    info("\n");
    exit(EXIT_FAILURE);
}
