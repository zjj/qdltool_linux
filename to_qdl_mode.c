#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include "generic.h" //xerror
#include "ext_libusb.h" 

unsigned char magic[] = {0x3a, 0xa1, 0x6e, 0x7e};
uint16_t _vendors[] = {0x18d1, 0x2717, 0x00};

static void usage()
{
    info("Usage:");
    info("  ./to_qdl_mode -l");
    info("  ./to_qdl_mode -s XXXXXXXX");
    exit(-1);
}

void try_switch_to_qdl(libusb_device_handle *handle)
{
    struct libusb_config_descriptor *conf_desc;
    const struct libusb_endpoint_descriptor *endpoint;
    struct libusb_interface_descriptor *altsetting;
    libusb_device *dev;
    int i, j, k, r;
    int interface_numbers = 0;
    dev = libusb_get_device(handle); 
    libusb_get_config_descriptor(dev, 0, &conf_desc);
    interface_numbers = conf_desc->bNumInterfaces;
    for(i=0; i<interface_numbers; i++){
        for(j=0; j<conf_desc->interface[i].num_altsetting; j++){
            for(k=0; k<conf_desc->interface[i].altsetting[j].bNumEndpoints; k++){
                endpoint = &conf_desc->interface[i].altsetting[j].endpoint[k];
                if(!(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN)){
                    r = libusb_claim_interface(handle, i); 
                    if (r != 0){
                        continue;
                    }
                    // try each endpoint since I dunno which on shall I write magic number into
                    int nil = 0;
                    r = libusb_bulk_transfer(handle,
                               endpoint->bEndpointAddress,
                               magic, sizeof(magic), &nil, 1000);
                    if(!r)
                        return;
                    libusb_release_interface(handle, i);
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    int r;
    libusb_device **devs;

    r = libusb_init(NULL);
    if (r < 0)
        return r;

    r = libusb_get_device_list(NULL, &devs);
    if (r < 0)
        return r;

    int opt;
    char serial[128] = {0};
    bool right_option = False;
    while((opt = getopt(argc, argv, "ls:")) != -1){
        if(opt == 'l'){
            right_option = True;
            print_devs(devs);
            break;
        }
        if(opt == 's'){
            right_option = True;
            strcpy(serial, optarg);
            libusb_device_handle *handle = NULL;
            if (serial[0])
                handle = get_device_handle_from_serial(serial, strlen(serial));
            if (!handle){
                printf("no such devices\n");
                break;
            }
            try_switch_to_qdl(handle);
            libusb_close(handle);
            break;
        }
    }

    if(!right_option){
        usage();
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return 0;
}
