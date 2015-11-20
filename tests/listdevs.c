/*
 * libusb example program to list devices on the bus
 * Copyright © 2007 Daniel Drake <dsd@gentoo.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <libusb.h>
#include <stdint.h>
typedef unsigned short int  uint16_t;
typedef unsigned char  uint8_t;

int main(void)
{
	int r, i;
	r = libusb_init(NULL);
	if (r < 0)
		return r;
    char serial[256] = {0}; 
    uint16_t    buffer[128] = {0};
    uint16_t    languages[128] = {0};
    int languageCount = 0;
    libusb_device_handle *devh = NULL;
    devh = libusb_open_device_with_vid_pid(NULL, 0x18d1, 0x4ee2);
    struct libusb_device_descriptor desc;
    libusb_device *dev = libusb_get_device(devh);
    libusb_get_device_descriptor(dev, &desc);
    libusb_detach_kernel_driver(devh, 0);
    printf("asdfasdf\n");
    memset(languages, 0, sizeof(languages));
    r = libusb_control_transfer(devh, 
        LIBUSB_ENDPOINT_IN |  LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
        LIBUSB_REQUEST_GET_DESCRIPTOR, LIBUSB_DT_STRING << 8, 0, (uint8_t *)languages, sizeof(languages), 0); 

    if (r <= 0) {
        printf("check_device(): Failed to get languages count\n");
    }   
        
    languageCount = (r - 2) / 2;
        
    for (i = 1; i <= languageCount; ++i) {
        memset(buffer, 0, sizeof(buffer));

        r = libusb_control_transfer(devh, 
            LIBUSB_ENDPOINT_IN |  LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
            LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_STRING << 8|desc.iSerialNumber), //check this from adb
    languages[i], (uint8_t *)buffer, sizeof(buffer), 0); 
        
        if (r > 0) { /* converting serial */
            int j = 0;
            r /= 2;

            for (j = 1; j < r; ++j)
                serial[j - 1] = buffer[j];

            serial[j - 1] = '\0';
            break; /* languagesCount cycle */
        }   
    }
    printf("%s\n", serial);
	libusb_exit(NULL);
	return 0;
}
