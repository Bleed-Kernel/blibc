#include <syscalls/exit.h>

__attribute__((noreturn))
void exit(int code){
    _exit(code);
    __builtin_unreachable();
}