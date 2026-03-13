#include <libc/wm.h>

#include <libc/ipc.h>
#include <libc/errno.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/syscalls/mmap.h>
#include <libc/syscalls/munmap.h>
#include <libc/syscalls/shm.h>
#include <libc/syscalls/yeild.h>

static int parse_u64(const char *s, uint64_t *out) {
    if (!s || !*s || !out)
        return -1;
    uint64_t v = 0;
    for (const char *p = s; *p; ++p) {
        if (*p < '0' || *p > '9')
            return -1;
        v = v * 10u + (uint64_t)(*p - '0');
    }
    *out = v;
    return 0;
}

int wm_parse_args(int argc, const char **argv, uint64_t *out_pid) {
    if (!out_pid)
        return -1;
    *out_pid = 0;
    if (!argv || argc <= 0)
        return -1;

    for (int i = 0; i + 1 < argc; ++i) {
        if (strcmp(argv[i], "--wm-pid") == 0) {
            uint64_t pid = 0;
            if (parse_u64(argv[i + 1], &pid) == 0) {
                *out_pid = pid;
                return 0;
            }
        }
    }
    return -1;
}

int wm_connect(wm_client *c, uint64_t pid) {
    if (!c || pid == 0)
        return -1;
    c->wm_pid = pid;
    c->connected = 1;
    return 0;
}

static int wm_send_msg(wm_client *c, const wm_msg *msg) {
    if (!c || !c->connected || !msg)
        return -1;
    void *page = _mmap(1);
    if (!page)
        return -1;
    memcpy(page, msg, sizeof(*msg));
    if (ipc_send((pid_t)c->wm_pid, page, 1) < 0) {
        _munmap(page);
        return -1;
    }
    return 0;
}

static int wm_recv_from(wm_client *c, wm_msg *out) {
    if (!c || !out)
        return -1;
    ipc_message_t msg;
    for (;;) {
        if (ipc_recv(&msg) == 0) {
            if (msg.pages == 0 || !msg.addr) {
                if (msg.addr)
                    _munmap((void *)(uintptr_t)msg.addr);
                continue;
            }

            wm_msg *in = (wm_msg *)(uintptr_t)msg.addr;
            if (msg.sender_pid != c->wm_pid || in->magic != WM_MSG_MAGIC) {
                _munmap((void *)(uintptr_t)msg.addr);
                continue;
            }
            *out = *in;
            _munmap((void *)(uintptr_t)msg.addr);
            return 0;
        }

        if (errno != EAGAIN)
            return -1;
        _yeild();
    }
}

int wm_create_window(wm_client *c, const char *title, int w, int h, wm_window *out_win) {
    if (!c || !out_win || w <= 0 || h <= 0)
        return -1;

    wm_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.magic = WM_MSG_MAGIC;
    msg.type = WM_MSG_CREATE;
    msg.width = (uint32_t)w;
    msg.height = (uint32_t)h;
    if (title && *title) {
        strncpy(msg.title, title, sizeof(msg.title) - 1);
        msg.title[sizeof(msg.title) - 1] = '\0';
    }

    wm_msg reply;
    int attempts = 0;
    for (;;) {
        if (wm_send_msg(c, &msg) < 0)
            return -1;
        if (wm_recv_from(c, &reply) < 0)
            return -1;
        if (reply.type == WM_MSG_CREATE_REPLY &&
            reply.shm_id != 0 && reply.window_id != 0) {
            break;
        }
        if (++attempts >= 50)
            return -1;
        _yeild();
    }

    void *addr = _shm_map(reply.shm_id);
    if (!addr)
        return -1;

    out_win->id = reply.window_id;
    out_win->width = reply.width;
    out_win->height = reply.height;
    out_win->shm_id = reply.shm_id;
    out_win->pixels = (uint32_t *)addr;
    return 0;
}

int wm_poll_event(wm_client *c, wm_window *win, wm_event *out_event) {
    if (!c || !win || !out_event)
        return -1;

    wm_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.magic = WM_MSG_MAGIC;
    msg.type = WM_MSG_POLL;
    msg.window_id = win->id;

    if (wm_send_msg(c, &msg) < 0)
        return -1;

    wm_msg reply;
    if (wm_recv_from(c, &reply) < 0)
        return -1;
    if (reply.type != WM_MSG_POLL_REPLY || reply.window_id != win->id)
        return -1;

    *out_event = reply.event;
    return 0;
}
