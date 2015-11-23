#include "qdl_usb.h"
#include "device.h"

#define EP_OUT 0x01
#define EP_IN  0x81

libusb_device_handle *handle = NULL;

int qdl_usb_init(char *serial)
{
    libusb_device *dev = NULL;
    libusb_device **devs;
    int r, ret;
    r = libusb_init(NULL);
    if (r < 0){
        xerror("libusb init error");
    }
    
    r = libusb_get_device_list(NULL, &devs);
    if (r < 0){
        printf("libusb_get_device_list error\n");
        libusb_exit(NULL);
        return r;
    }

    if(serial[0]){
        dev = get_device_from_serial(serial);
        if(!dev){
            printf("device not legal, plese run -l to check\n");
            return -1;
        }
        if(!is_legal_qdl_device(dev)){
            printf("device not legal, plese run -l to check\n");
            ret = -1;
            goto final;
        }
    }else{  //for without -s, try to get the default device
        int matched = check_qdl_devices(devs, &dev);
        if (matched == 0){
            printf("there's no legal devie\n");
            return -1;
        }
        if(matched > 1){
            printf("there's more than one qdl device, plase add -s XXXXX to specify one\n");
            return -1;
        }
    }
    if(dev == NULL){
        printf("failed to get dev while qdl_usb_init\n");
        ret = -1;
        goto final;
    }
    libusb_open(dev, &handle);
    if(!handle){
        printf("libusb_open error\n");
        ret = -1;
        goto final;
    }
    libusb_detach_kernel_driver(handle, 0); //interface 0
    r = libusb_claim_interface(handle, 0);  //interface 0
    if (r != 0){
        printf("usb claim interface error");
        ret = -1;
        goto final;
    }

final:
    if(dev)
        libusb_unref_device(dev);
    libusb_free_device_list(devs, 1);
    if(ret < 0){
        if (handle) 
            libusb_close(handle);
        libusb_exit(NULL);
    }
    return ret;
}

void print_all_qdl_devices() //for -l or --list olny
{
    libusb_device **devs;
    int r, ret;
    r = libusb_init(NULL);
    if (r < 0){
        xerror("libusb init error");
    }
    
    r = libusb_get_device_list(NULL, &devs);
    if (r < 0){
        printf("libusb_get_device_list error\n");
        return;
    }

    print_qdl_devs(devs);
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
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

    ret = libusb_bulk_transfer(handle, EP_OUT, buf, len, count, 1000*10);
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
    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(NULL);
}
