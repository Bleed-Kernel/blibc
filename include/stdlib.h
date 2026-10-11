#pragma once
#include <stdint.h>
#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int atoi(const char *s);
double atof(const char *s);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);
void abort(void) __attribute__((noreturn));

void free(void* ptr);
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void* calloc(size_t nmemb, size_t size);

int abs(int x);
int system(const char *command);

char *getenv(const char *name);

void exit(int code) __attribute__((noreturn));
