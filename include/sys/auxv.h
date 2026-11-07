#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <elf.h>

unsigned long getauxval(unsigned long type);

#ifdef __cplusplus
}
#endif
