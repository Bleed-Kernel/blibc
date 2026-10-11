#pragma once
#include <stdint.h>

#define AUDIO_IOCTL_GET_SETTINGS  0x4100
#define AUDIO_IOCTL_SET_SETTINGS  0x4101
#define AUDIO_IOCTL_RESET         0x4102

typedef struct {
    uint32_t sample_rate;
    uint8_t  volume_left;
    uint8_t  volume_right;
    uint8_t  muted;
} audio_settings_t;