#include <stdlib.h>

static unsigned int state = 1;

void srand(unsigned int seed) {
    state = seed;
}

int rand(void) {
    state = state * 1103515245U + 12345U;
    return (int)((state >> 16) & RAND_MAX);
}
