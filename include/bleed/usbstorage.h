#pragma once
#include <stdint.h>

#define USB_STORAGE_IOC_INFO    0x5501
#define USB_STORAGE_IOC_EJECT   0x5502

struct usb_storage_info {
    char     vendor[9];
    char     product[17];
    char     revision[5];
    char     _pad[1];
    char     mount[64];         // where it is mounted, empty if it isnt
    uint64_t sectors;
    uint32_t sector_size;
    uint32_t _pad2;
};
