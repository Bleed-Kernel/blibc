#pragma once

#include <libc/stdint.h>
#include <libc/types.h>
#include <libc/stddef.h>

typedef struct ipc_message {
    uint64_t sender_pid;
    uint64_t addr;
    uint64_t pages;
    uint64_t flags;
} ipc_message_t;

int ipc_send(pid_t target_pid, void *addr, size_t pages);
int ipc_recv(ipc_message_t *msg);
