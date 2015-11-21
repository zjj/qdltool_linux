#include <stdio.h>
#include <libusb.h>
#include "generic.h"

static uint16_t qualcomm_vendors[] = {0x18d1, 0x00};

bool is_matched_device(struct libusb_device_descriptor desc)
{
    uint16_t *p = qualcomm_vendors;
    while(*p != 0){
        if(desc.idVendor == *p)
            return True;
        p++;
    }
    return False;
}

static void print_devs(libusb_device **devs)
{
    libusb_device *dev;

    int d = 0;
    while ((dev = devs[d++]) != NULL) {
        char serial[256] = {0};
        uint16_t languages[128] = {0};
        int languageCount = 0;

        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if(is_matched_device(desc)){
            libusb_device_handle *handle = NULL;
            libusb_open(dev, &handle);
            if(handle == NULL){
                xerror("usb open error");
            }  
            libusb_detach_kernel_driver(handle, 0);
            r = libusb_control_transfer(handle, 
                                        LIBUSB_ENDPOINT_IN |  LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
                                        LIBUSB_REQUEST_GET_DESCRIPTOR, LIBUSB_DT_STRING << 8, 0,
                                        (uint8_t *)languages, sizeof(languages), 0); 
            if (r <= 0) {
                printf("check_device(): Failed to get languages count\n");
            }
            languageCount = (r - 2) / 2;

            int i;
            for (i = 1; i <= languageCount; ++i) {
                uint16_t buffer[128] = {0};

                r = libusb_control_transfer(handle,
                                            LIBUSB_ENDPOINT_IN |  LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
                                            LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_STRING << 8|desc.iSerialNumber),
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
        }
    }
}

int main(void)
{
    libusb_device **devs;
    int r;
    ssize_t cnt;

    r = libusb_init(NULL);
    if (r < 0)
        return r;

    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0)
        return (int) cnt;

    print_devs(devs);
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return 0;
}
