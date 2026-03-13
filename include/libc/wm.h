#pragma once

#include <libc/stdint.h>
#include <libc/types.h>

#define WM_MSG_MAGIC 0x574D5347u

typedef enum {
    WM_EVENT_NONE = 0,
    WM_EVENT_MOUSE_MOVE,
    WM_EVENT_MOUSE_DOWN,
    WM_EVENT_MOUSE_UP,
    WM_EVENT_KEY_DOWN,
    WM_EVENT_KEY_UP,
    WM_EVENT_TEXT_INPUT
} wm_event_type;

typedef struct {
    wm_event_type type;
    int mouse_x;
    int mouse_y;
    uint8_t mouse_button;
    uint32_t key;
    char text;
} wm_event;

typedef struct {
    uint32_t magic;
    uint16_t type;
    uint16_t size;
    uint32_t window_id;
    uint32_t width;
    uint32_t height;
    uint64_t shm_id;
    wm_event event;
    char title[32];
} wm_msg;

enum {
    WM_MSG_CREATE = 1,
    WM_MSG_CREATE_REPLY = 2,
    WM_MSG_POLL = 3,
    WM_MSG_POLL_REPLY = 4,
    WM_MSG_CLOSE = 5
};

typedef struct {
    uint64_t wm_pid;
    int connected;
} wm_client;

typedef struct {
    uint32_t id;
    uint32_t width;
    uint32_t height;
    uint64_t shm_id;
    uint32_t *pixels;
} wm_window;

int wm_parse_args(int argc, const char **argv, uint64_t *out_pid);
int wm_connect(wm_client *c, uint64_t pid);
int wm_create_window(wm_client *c, const char *title, int w, int h, wm_window *out_win);
int wm_poll_event(wm_client *c, wm_window *win, wm_event *out_event);
