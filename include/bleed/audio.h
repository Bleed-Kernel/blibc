#pragma once
#include <stdint.h>

#define AUDIO_FORMAT_S16LE   1

#define AUDIO_IOC_GET_INFO   0x5101  // struct audio_info out
#define AUDIO_IOC_ACQUIRE    0x5102  // struct audio_map out, maps the ring into the caller
#define AUDIO_IOC_RELEASE    0x5103
#define AUDIO_IOC_START      0x5104
#define AUDIO_IOC_STOP       0x5105  // stops and drops anything queued
#define AUDIO_IOC_GET_POS    0x5106  // audio_pos
#define AUDIO_IOC_COMMIT     0x5107  // uint64_t bytes in, how much was written at write_pos
#define AUDIO_IOC_SET_RATE   0x5108  // uint32_t in/out, stopped only, out is what the codec took
#define AUDIO_IOC_SET_VOLUME 0x5109  // struct audio_volume in, anyone can, its the cards own volume
#define AUDIO_IOC_GET_OWNER  0x510A  // pid
#define AUDIO_IOC_WAIT       0x510B  // sleep till the hardware finishes the period its on
#define AUDIO_IOC_GET_VOLUME 0x510C  // struct audio_volume out, what SET_VOLUME last set

struct audio_info {
    uint32_t rate;
    uint32_t channels;
    uint32_t format;
    uint32_t frame_bytes;
    uint32_t period_bytes;
    uint32_t periods;
    uint64_t ring_bytes;
};

struct audio_map {
    uint64_t addr;
    uint64_t ring_bytes;
};

// bytes since START, ring offset is pos % ring_bytes
struct audio_pos {
    uint64_t write_pos;
    uint64_t writable;
    uint64_t played;
    uint64_t hw_pos;
    uint64_t underruns;
    uint32_t running;
    uint32_t _pad;
};

struct audio_volume {
    uint8_t left;       // 0 - 100
    uint8_t right;
    uint8_t mute;
    uint8_t _pad;
};
