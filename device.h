#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "global.h"

extern bool is_legal_device(libusb_device *dev);
extern bool is_legal_qdl_device(libusb_device *dev);
extern int get_device_serial(libusb_device *dev, char *serial);
extern libusb_device *get_device_from_serial(char *serial);
extern libusb_device_handle *get_device_handle_from_serial(char *serial);
extern void print_devs(libusb_device **devs);
extern void print_qdl_devs();

#endif
