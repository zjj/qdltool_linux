#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include "device.h" 

unsigned char magic[] = {0x3a, 0xa1, 0x6e, 0x7e};

static void usage()
{
    info("Usage:");
    info("  ./to_qdl_mode -l");
    info("  ./to_qdl_mode -s XXXXXXXX");
    exit(-1);
}

void switch_to_qdl_mode(libusb_device_handle *handle)
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
                    printf("lalalalal\n");
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
    char serial[256] = {0};
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
            libusb_device *dev = NULL;
            libusb_device_handle *handle = NULL;
            if (serial[0]){
                dev = get_device_from_serial(serial);
                if(!dev){
                    printf("device not legal, please run -l to check\n");
                    break;
                }
                if (!is_legal_device(dev)){
                    printf("device not legal, please run -l to check\n");
                    break;
                }
                libusb_open(dev, &handle);
                if (!handle){
                    printf("no such devices\n");
                    break;
                }
                switch_to_qdl_mode(handle);
                libusb_close(handle);
                libusb_unref_device(dev);
                break;
            }
        }
    }

    if(argc == 1){ //try default
        libusb_device *candy;
        int matched = check_devices(devs, &candy);
        if (matched == 1){
            libusb_device_handle *handle = NULL;
            libusb_open(candy, &handle);
            if (handle)
                switch_to_qdl_mode(handle);
            libusb_unref_device(candy);
            libusb_close(handle);
        }
        if (matched < 1)
            info("there's no legal device");

        if (matched > 1){
            info("I don't know which device to choose, so many devices, please -s XXXXX to specify one from below");
            print_devs(devs);
        }
    }

    if(argc > 1 && !right_option){
        usage();
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return 0;
}
