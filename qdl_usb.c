#include "qdl_usb.h"

#define EP_OUT 0x01
#define EP_IN  0x81

libusb_device_handle *handle = NULL;

int qdl_usb_init(char *serial)
{
    libusb_device *dev = NULL;
    libusb_device **devs = NULL;
    int  r, ret = 0;
    do{
        r = libusb_init(NULL);
        if (r < 0){
            xerror("libusb init error");
            ret = -1;
            break;
        }

        r = libusb_get_device_list(NULL, &devs);
        if (r < 0){
            printf("libusb_get_device_list error\n");
            ret = -1;
            break;
        }

        if(serial[0]){  //for -s
            dev = get_device_from_serial(serial);
            if(!dev){
                printf("device not legal, plese run -l to check\n");
                ret = -1;
                break;
            }
            if(!is_legal_qdl_device(dev)){
                printf("device not legal, plese run -l to check\n");
                ret = -1;
                break;
            }
        }else{  //for without -s, try to get the default device
            int matched = check_qdl_devices(devs, &dev);
            if (matched < 1){
                printf("there's no legal devie\n");
                ret = -1;
                break;
            }
            if(matched > 1){
                printf("there's more than one qdl device, plase add -s XXXXX to specify one\n");
                ret = -2;
                break;
            }
        }
        if(dev == NULL){
            printf("failed to get dev while qdl_usb_init\n");
            ret = -1;
            break;
        }
        libusb_open(dev, &handle);
        if(!handle){
            printf("libusb_open error\n");
            ret = -1;
            break;
        }
        libusb_detach_kernel_driver(handle, 0); //interface 0
        r = libusb_claim_interface(handle, 0);  //interface 0
        if (r != 0){
            printf("usb claim interface error");
            ret = -1;
            break;
        }
    }while(0);

    if(ret < 0){
        if(handle)
            libusb_close(handle);
        if(dev)
            libusb_unref_device(dev);
        if(devs)
            libusb_free_device_list(devs, 1);
        libusb_exit(NULL);
    }
    return ret;
}

int write_tx(void *buf, int len, int *act)
{
    int ret;
    int nil;
    int max_retry = 5;
    int *count = act;
    if (!count)
        count = &nil;
    *count = 0;

    ret = libusb_bulk_transfer(handle, EP_OUT, buf, len, count, 1000*20);
    return ret;
/*
    do{
        ret  = libusb_bulk_transfer(handle, EP_OUT, buf, len, count, 5000);
        if (ret == LIBUSB_ERROR_TIMEOUT){   // && *count == 0){
            usleep(500);
            continue;
        }else{
            if (ret == LIBUSB_ERROR_PIPE){  // && *count== 0) {
                libusb_clear_halt(handle, EP_OUT);
                continue;
            }else{
                return ret;
            }
        }
    }while(max_retry--);
*/
}

int read_rx_timeout(void *buf, int length, int *act, int timeout)
{
    int ret;
    int nil;
    int max_retry = 3;
    int *count = act;
    if (!count)
        count = &nil;
    *count = 0;
    return libusb_bulk_transfer(handle, EP_IN, buf, length, count, timeout);
    /*
    do{
        ret = libusb_bulk_transfer(handle, EP_IN, buf, length, count, timeout);
        if (ret == LIBUSB_ERROR_TIMEOUT){
            usleep(500);
            continue;
        }else{
            if (ret == LIBUSB_ERROR_PIPE){  // && *count == 0) {
                libusb_clear_halt(handle, EP_IN);
                continue;
            }else{
                return ret;
            }
        }
    }while(max_retry--);
    */
}

int read_rx(void *buf, int length, int *act)
{
    return read_rx_timeout(buf, length, act, 1000); //1s timeout default
}

void qdl_usb_close()
{
    if(handle){
        libusb_release_interface(handle, 0);
        libusb_close(handle);
        handle = NULL;
    }
    libusb_exit(NULL);
}
