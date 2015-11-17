#ifndef __MISC_H_
#define __MISC_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

extern size_t get_file_size(int fd, size_t *size);
extern void print_stage_info(char *s);
extern void usage();

#endif
