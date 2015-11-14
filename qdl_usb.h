#ifndef QDL_USB_H_
#define QDL_USB_H_
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>

extern int qdl_usb_init();
extern int write_tx(void *buf, int length, int *act);
extern int read_rx(void *buf, int length, int *act);
extern void qdl_usb_close();
extern int read_rx_timeout(void *buf, int length, int *act, int timeout);

#endif
