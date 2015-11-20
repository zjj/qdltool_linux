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
