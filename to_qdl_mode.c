#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include "generic.h" //xerror

static void usage()
{
    info("Usage:");
    info("  ./to_qdl_mode id_vendor:id_product");
    info("you could get id_vendor:id_product from lsusb");
    exit(-1);
}

int main(int argc, char **argv)
{

    if(argc != 2){
        usage();
    }
    char *device = argv[1];
    if(!strchr(device, ':')){
        usage();
    }

    u32 id_vendor = 0;
    u32 id_product = 0;
    
    sscanf(device, "%04x:%04x", &id_vendor, &id_product);
    if(!(id_vendor && id_product)){
        usage();
    }
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
        xerror("libusb init error");
    }   
    handle = libusb_open_device_with_vid_pid(NULL, id_vendor, id_product);
    if (handle == NULL){
        libusb_exit(NULL);
        xerror("usb error");
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
