#ifndef __MISC_H_
#define __MISC_H_

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

#ifdef DEBUG
#define LOG(format, ...) fprintf(stdout, "LOG "format"\n", ##__VA_ARGS__)
#else
#define LOG(format, ...)
#endif

#define xerror(format, ...) do{fprintf(stderr, "ERROR "format"\n", ##__VA_ARGS__);exit(-1);}while(0)
#define info(format, ...) do{fprintf(stdout, format"\n", ##__VA_ARGS__), fflush (stdout);}while(0)

#define MAX_LENGTH (1024*1024) //1M
#define MAX_RETRY 30
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

extern size_t get_file_size(int fd, size_t *size);
extern void print_stage_info(char *s);
extern bool is_legal_device(libusb_device *dev);
extern bool is_legal_qdl_device(libusb_device *dev);
extern int get_device_serial(libusb_device *dev, char *serial);
extern libusb_device *get_device_from_serial(char *serial);
extern libusb_device_handle *get_device_handle_from_serial(char *serial);
extern void print_devs(libusb_device **devs);
extern void print_qdl_devs(libusb_device **devs);

#endif
