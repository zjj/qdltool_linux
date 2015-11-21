#ifndef _EXT_LIBUSB_H_
#define _EXT_LIBUSB_H_
#include "generic.h" //xerror
#include <libusb.h>

extern uint16_t _vendors[];
extern bool is_matched_device(libusb_device *dev, uint16_t *vendors);
extern int get_device_serial(libusb_device *dev, char *serial);
extern libusb_device_handle *get_device_handle_from_serial(char *serial, int len);
extern void print_devs(libusb_device **devs);
extern void try_switch_to_qdl(libusb_device_handle *handle);
#endif
