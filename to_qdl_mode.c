#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>

#define VENDOR_ID 0x1bbb
#define PRODUCT_ID 0xaf02
 

int main()
{
    unsigned char magic[] = {0x3a, 0xa1, 0x6e, 0x7e};
    int nil;
    int r;
    int nb_ifaces;
    libusb_device_handle *handle;
    libusb_device *dev;
    struct libusb_config_descriptor *conf_desc;
    const struct libusb_endpoint_descriptor *endpoint;
    struct libusb_interface_descriptor *altsetting;
    int i, j, k;
    const struct libusb_interface *interface;
    int interface_numbers;
    unsigned char endpoint_address[128] = {0};
    unsigned char *ptr = endpoint_address;

    r = libusb_init(NULL);
    if (r != 0){ 
        printf("libusb init error");
        exit(-1);
    }   
    handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
    if (handle == NULL){
        printf("usb error");
        exit(-1);
    }

    dev = libusb_get_device(handle); 
    libusb_get_config_descriptor(dev, 0, &conf_desc);
    interface_numbers = conf_desc->bNumInterfaces;
    for(i=0; i<interface_numbers; i++){
        for(j=0; j<conf_desc->interface[i].num_altsetting; j++){
            for(k=0; k<conf_desc->interface[i].altsetting[j].bNumEndpoints; k++){
                endpoint = &conf_desc->interface[i].altsetting[j].endpoint[k];
                if(!(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN)){
                    libusb_detach_kernel_driver(handle, i);
                    r = libusb_claim_interface(handle, i);
                    if (r != 0){
                        continue;
                    }
                    /* try each endpoint since I dunno which on shall I write magic number into */
                    r = libusb_bulk_transfer(handle,
                                             endpoint->bEndpointAddress,
                                             magic, sizeof(magic), &nil, 1000);
                    libusb_release_interface(handle, i);
                }
            }
        }
    }
    libusb_close(handle);
    libusb_exit(NULL);
}
