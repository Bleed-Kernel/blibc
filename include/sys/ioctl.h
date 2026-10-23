#pragma once

#include <termios.h>

int ioctl(int fd, unsigned long request, void *arg);
