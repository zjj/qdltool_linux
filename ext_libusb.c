#include "ext_libusb.h"

bool is_matched_device(libusb_device *dev, uint16_t *vendors)
{
    int r;
    uint16_t *p = vendors;
    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r)
        return False;

    while(*p != 0){
        if(desc.idVendor == *p)
            return True;
        p++;
    }
    return False;
}

int get_device_serial(libusb_device *dev, char *serial)
{
    int r = 0;
    uint16_t languages[128] = {0};
    int languageCount = 0;

    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0)
        return r;
    libusb_device_handle *handle = NULL;
    libusb_open(dev, &handle);
    if(handle == NULL){
        //xerror("usb open error");
        return r;
    }  
    r = libusb_control_transfer(handle, 
            LIBUSB_ENDPOINT_IN |  LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
            LIBUSB_REQUEST_GET_DESCRIPTOR, LIBUSB_DT_STRING << 8, 0,
            (uint8_t *)languages, sizeof(languages), 0); 

    if (r <= 0) {
        printf("check_device(): Failed to get languages count\n");
        return r;
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
        if(r<0){
            libusb_close(handle);
            return r;
        }
    }
    libusb_close(handle);
    return 0;
}

libusb_device_handle *get_device_handle_from_serial(char *ser, int len)
{
    int r;
    libusb_device *dev;
    libusb_device **devs;
    r = libusb_get_device_list(NULL, &devs);
    if (r < 0)
        xerror("libusb_get_device_list error");

    int d = 0;
    while ((dev = devs[d++]) != NULL) {
        char serial[256] = {0};
        uint16_t languages[128] = {0};
        int languageCount = 0;

        if(is_matched_device(dev, _vendors)){
            char serial[256] = {0};
            libusb_device_handle *handle = NULL;
            libusb_open(dev, &handle);
            if(!handle)
                continue;
            get_device_serial(dev, serial);
            if (!strncmp(ser, serial, len)){
                libusb_free_device_list(devs, 1);
                return handle;    
            }
            libusb_close(handle);
        }
    }

    libusb_free_device_list(devs, 1);
    return NULL;
}

void print_devs(libusb_device **devs)
{
    libusb_device *dev;

    int d = 0;
    while ((dev = devs[d++]) != NULL) {
        char serial[256] = {0};
        if(is_matched_device(dev, _vendors)){
            if(!get_device_serial(dev, serial))
                printf("%s\n", serial);
        }
    }
}
