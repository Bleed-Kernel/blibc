#include <stdint.h>
#include <stdlib.h>

static unsigned int state = 1;

void srand(unsigned int seed) {
    state = seed;
}

int rand(void) {
    state = state * 1103515245U + 12345U;
    return (int)((state >> 16) & RAND_MAX);
}

static uint64_t random_state = 1;

void srandom(unsigned int seed) {
    random_state = seed;
}

long random(void) {
    random_state = random_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return (long)(random_state >> 33);
}

char *initstate(unsigned int seed, char *state_buf, size_t n) {
    (void)state_buf; (void)n;
    srandom(seed);
    return state_buf;
}

char *setstate(char *state_buf) {
    return state_buf;
}

static unsigned short seed48_state[3] = { 0x330e, 0, 0 };

static uint64_t rand48_step(unsigned short x[3]) {
    uint64_t v = x[0] | ((uint64_t)x[1] << 16) | ((uint64_t)x[2] << 32);
    v = (v * 0x5deece66dULL + 0xb) & 0xffffffffffffULL;
    x[0] = (unsigned short)v;
    x[1] = (unsigned short)(v >> 16);
    x[2] = (unsigned short)(v >> 32);
    return v;
}

double erand48(unsigned short x[3]) { return (double)rand48_step(x) / 281474976710656.0; }
double drand48(void)                 { return erand48(seed48_state); }
long nrand48(unsigned short x[3])    { return (long)(rand48_step(x) >> 17); }
long lrand48(void)                   { return nrand48(seed48_state); }
long jrand48(unsigned short x[3])    { return (long)(int32_t)(rand48_step(x) >> 16); }
long mrand48(void)                   { return jrand48(seed48_state); }

void srand48(long seed) {
    seed48_state[0] = 0x330e;
    seed48_state[1] = (unsigned short)seed;
    seed48_state[2] = (unsigned short)(seed >> 16);
}
