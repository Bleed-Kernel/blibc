#pragma once

#include <stdint.h>

#define TOUCH_MAX 0xFFFF

enum {
    TOUCH_FINGER = 1 << 0,
    TOUCH_DOWN   = 1 << 1 
};

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t  contact;
    uint8_t  flags;
    uint8_t  buttons;
    int8_t   wheel;
} touch_event_t;
