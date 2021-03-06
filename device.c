#include "device.h"

bool is_legal_device(libusb_device *dev) //check for switch_to_qdl_mode
{
    int r;
    uint16_t vendors[] = {0x1bbb, 0x00};
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

bool is_legal_qdl_device(libusb_device *dev) //check for 9008
{
    int r;
    uint16_t vendors[] = {0x05c6, 0x00};
    uint16_t products[] = {0x9008, 0x00};
    uint16_t *v = vendors;
    uint16_t *p = products;
    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r)
        return False;
    while((*v != 0) && (*p != 0)){
        if(desc.idVendor == *v && desc.idProduct == *p)
            return True;
        v++;
        p++;
    }
    return False;
}

int get_device_serial(libusb_device *dev, char *serial)
{
    int r = 0;
    char serial1[256] = {0};
    char serial2[256] = {0};
    uint16_t languages[128] = {0};
    int languageCount = 0;

    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0)
        return r;

    int bus_number = 0;
    int dev_address = 0;
    char path[8] = {0};

    bus_number = libusb_get_bus_number(dev);
    dev_address = libusb_get_device_address(dev);

    r = libusb_get_port_numbers(dev, path, sizeof(path));
    if (r > 0) {
        sprintf(serial1, "%04x%04x%d%d%d", desc.idVendor, desc.idProduct, bus_number, dev_address, path[0]);
    }else{
        return -1;
    }

    libusb_device_handle *handle = NULL;
    libusb_open(dev, &handle);
    if(handle == NULL){
        //xerror("usb open error");
        return r;
    }

    r = libusb_control_transfer(handle,
            LIBUSB_ENDPOINT_IN |  LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
            LIBUSB_REQUEST_GET_DESCRIPTOR, LIBUSB_DT_STRING << 8, 0,
            (uint8_t *)languages, sizeof(languages), 1000);

    if (r <= 0){
        return r;
    }
    languageCount = (r - 2) / 2;

    int i;
    for (i = 1; i <= languageCount; ++i) {
        uint16_t buffer[128] = {0};

        r = libusb_control_transfer(handle,
                LIBUSB_ENDPOINT_IN |  LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
                LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_STRING << 8|desc.iSerialNumber),
                languages[i], (uint8_t *)buffer, sizeof(buffer), 1000);

        if (r >= 0) { /* converting serial */
            int j = 0;
            r /= 2;
            for (j = 1; j < r; ++j)
                serial2[j - 1] = buffer[j];

            serial2[j - 1] = '\0';
            break; /* languagesCount cycle */
        }

        if(r < 0){
            break;
        }
    }
    if(serial2[0])
        strcpy(serial, serial2);
    else
        strcpy(serial, serial1);
    libusb_close(handle);
    return 0;
}

libusb_device *get_device_from_serial(char *ser)
{
    int r;
    libusb_device *dev = NULL;
    libusb_device **devs;
    r = libusb_get_device_list(NULL, &devs);
    if (r < 0){
        printf("libusb_get_device_list error");
        return NULL;
    }
    int d = 0;
    while ((dev = devs[d++]) != NULL) {
        char serial[256] = {0};
        r = get_device_serial(dev, serial);
        if(r < 0)
            continue;
        if (!strncmp(ser, serial, strlen(ser))){
            libusb_ref_device(dev);
            break;
        }
    }

    libusb_free_device_list(devs, 1);
    return dev;
}


void print_devs(libusb_device **devs)
{
    libusb_device *dev;

    int d = 0;
    while ((dev = devs[d++]) != NULL) {
        char serial[256] = {0};
        if(is_legal_device(dev)){
            if(!get_device_serial(dev, serial))
                printf("%s\n", serial);
        }
    }
}

void print_qdl_devices() //for -l or --list olny
{
    libusb_device *dev;
    libusb_device **devs;
    int r;
    r = libusb_init(NULL);
    if (r < 0){
        printf("print_qdl_devices libusb_init error\n");
        return;
    }

    r = libusb_get_device_list(NULL, &devs);
    if (r < 0){
        printf("print_qdl_devices libusb_get_device_list error\n");
        libusb_exit(NULL);
        return;
    }

    int d = 0;
    while ((dev = devs[d++]) != NULL) {
        char serial[256] = {0};
        if(is_legal_qdl_device(dev)){
            if(!get_device_serial(dev, serial))
                printf("%s\n", serial);
        }
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
}

int check_devices(libusb_device **devs, libusb_device **candy)
{
    libusb_device *dev;
    int d = 0;
    int matched = 0;
    while ((dev = devs[d++]) != NULL) {
        if(is_legal_device(dev)){
            matched ++;
            *candy = dev;
        }
    }
    if(matched == 1)
        libusb_ref_device(*candy);
    return matched;
}

int check_qdl_devices(libusb_device **devs, libusb_device **candy)
{
    libusb_device *dev;
    int d = 0;
    int matched = 0;
    while ((dev = devs[d++]) != NULL) {
        if(is_legal_qdl_device(dev)){
            matched ++;
            *candy = dev;
        }
    }
    if(matched == 1)
        libusb_ref_device(*candy);
    return matched;
}
