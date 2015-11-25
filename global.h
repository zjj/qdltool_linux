#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <libusb.h>
#include <getopt.h>

#ifdef DEBUG
#define LOG(format, ...) fprintf(stdout, "LOG "format"\n", ##__VA_ARGS__)
#else
#define LOG(format, ...)
#endif

#define xerror(format, ...) do{fprintf(stderr, "ERROR "format"\n", ##__VA_ARGS__);exit(-1);}while(0)
#define info(format, ...) do{fprintf(stdout, format"\n", ##__VA_ARGS__), fflush (stdout);}while(0)

#define MAX_LENGTH (1024*1024) //1M
#define MAX_RETRY 1000
#define FIREHOSE_VERBOSE "0"

typedef unsigned int u32, uint32;
typedef unsigned char u8, byte;
typedef unsigned long long int SIZE_T;
typedef enum {
    FALSE=0,
    False=0,
    TRUE=1,
    True=1
} boolean, bool;

#endif
