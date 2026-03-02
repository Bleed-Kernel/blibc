#pragma once

#include <libc/ipc.h>

long _ipc_send(uint64_t target_pid, uint64_t src_addr, uint64_t pages);
long _ipc_recv(ipc_message_t *msg);
