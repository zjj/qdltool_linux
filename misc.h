#ifndef __MISC_H_
#define __MISC_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>
#include "generic.h"

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
