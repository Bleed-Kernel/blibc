#pragma once

#include <libc/stdint.h>

long _exec(const char *path, const char *const argv[], uint64_t argc);
