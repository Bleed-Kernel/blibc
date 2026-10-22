#include <unistd.h>
#include <time.h>

unsigned int sleep(unsigned int seconds) {
    struct timespec ts = { (time_t)seconds, 0 };
    return nanosleep(&ts, NULL) == 0 ? 0 : seconds;
}

int usleep(unsigned long usec) {
    struct timespec ts = { (time_t)(usec / 1000000), (long)(usec % 1000000) * 1000 };
    return nanosleep(&ts, NULL);
}
