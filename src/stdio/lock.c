#include <stdio.h>
#include <sys/types.h>
#include "../thread/thread.h"

// recursive, the locked calls below lean on each other
void flockfile(FILE *f) {
    int tid = thread_self()->tid;
    if (f->lock_owner == tid) {
        f->lock_count++;
        return;
    }
    lock_take(&f->lock);
    f->lock_owner = tid;
    f->lock_count = 1;
}

int ftrylockfile(FILE *f) {
    int tid = thread_self()->tid;
    if (f->lock_owner == tid) {
        f->lock_count++;
        return 0;
    }
    if (!__sync_bool_compare_and_swap(&f->lock, 0, 1))
        return -1;
    f->lock_owner = tid;
    f->lock_count = 1;
    return 0;
}

void funlockfile(FILE *f) {
    if (--f->lock_count > 0)
        return;
    f->lock_owner = 0;
    lock_give(&f->lock);
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *f) {
    if (!f)
        return 0;
    flockfile(f);
    size_t r = fread_unlocked(ptr, size, nmemb, f);
    funlockfile(f);
    return r;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *f) {
    if (!f)
        return 0;
    flockfile(f);
    size_t r = fwrite_unlocked(ptr, size, nmemb, f);
    funlockfile(f);
    return r;
}

char *fgets(char *s, int size, FILE *f) {
    if (!f)
        return NULL;
    flockfile(f);
    char *r = fgets_unlocked(s, size, f);
    funlockfile(f);
    return r;
}

int fgetc(FILE *f) {
    if (!f)
        return EOF;
    flockfile(f);
    int r = fgetc_unlocked(f);
    funlockfile(f);
    return r;
}

int ungetc_unlocked(int c, FILE *f);
int ungetc(int c, FILE *f) {
    if (!f)
        return EOF;
    flockfile(f);
    int r = ungetc_unlocked(c, f);
    funlockfile(f);
    return r;
}

ssize_t getdelim_unlocked(char **line, size_t *cap, int delim, FILE *f);
ssize_t getdelim(char **line, size_t *cap, int delim, FILE *f) {
    if (!f)
        return getdelim_unlocked(line, cap, delim, f);
    flockfile(f);
    ssize_t r = getdelim_unlocked(line, cap, delim, f);
    funlockfile(f);
    return r;
}

int fseek_unlocked(FILE *f, long offset, int whence);
int fseek(FILE *f, long offset, int whence) {
    if (!f)
        return -1;
    flockfile(f);
    int r = fseek_unlocked(f, offset, whence);
    funlockfile(f);
    return r;
}

int getc_unlocked(FILE *f) {
    return fgetc_unlocked(f);
}

int getchar_unlocked(void) {
    return fgetc_unlocked(stdin);
}

int fputc_unlocked(int c, FILE *f) {
    unsigned char ch = (unsigned char)c;
    return fwrite_unlocked(&ch, 1, 1, f) == 1 ? ch : EOF;
}

int putc_unlocked(int c, FILE *f) {
    return fputc_unlocked(c, f);
}

int putchar_unlocked(int c) {
    return fputc_unlocked(c, stdout);
}

int fputs_unlocked(const char *s, FILE *f) {
    return fputs(s, f);
}

int fflush_unlocked(FILE *f) {
    return fflush(f);
}

int feof_unlocked(FILE *f) {
    return feof(f);
}

int ferror_unlocked(FILE *f) {
    return ferror(f);
}

void clearerr_unlocked(FILE *f) {
    clearerr(f);
}

int fileno_unlocked(FILE *f) {
    return fileno(f);
}
