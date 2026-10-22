#include <stdlib.h>

#define SMALL   12

static void swap(unsigned char *a, unsigned char *b, size_t size) {
    while (size--) {
        unsigned char t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

static void insertion(unsigned char *base, size_t n, size_t size, int (*cmp)(const void *, const void *)) {
    for (size_t i = 1; i < n; i++) {
        for (size_t j = i; j > 0 && cmp(base + (j - 1) * size, base + j * size) > 0; j--)
            swap(base + (j - 1) * size, base + j * size, size);
    }
}

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
    unsigned char *lo = base;
    size_t         n  = nmemb;

    if (!base || !compar || size == 0)
        return;

    while (n > SMALL) {
        unsigned char *mid  = lo + (n / 2) * size;
        unsigned char *last = lo + (n - 1) * size;
        if (compar(mid, lo) < 0)   swap(mid, lo, size);
        if (compar(last, lo) < 0)  swap(last, lo, size);
        if (compar(last, mid) < 0) swap(last, mid, size);

        // the pivot sits at the end while partitioning so its never moved under us
        swap(mid, last - size, size);
        unsigned char *pivot = last - size;

        unsigned char *i = lo;
        unsigned char *j = pivot;
        for (;;) {
            do i += size; while (compar(i, pivot) < 0);
            do j -= size; while (j > lo && compar(j, pivot) > 0);
            if (i >= j)
                break;
            swap(i, j, size);
        }
        swap(i, pivot, size);

        size_t left  = (size_t)(i - lo) / size;
        size_t right = n - left - 1;
        if (left < right) {
            qsort(lo, left, size, compar);
            lo = i + size;
            n  = right;
        } else {
            qsort(i + size, right, size, compar);
            n = left;
        }
    }
    insertion(lo, n, size, compar);
}

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *)) {
    const unsigned char *lo = base;
    while (nmemb > 0) {
        const unsigned char *mid = lo + (nmemb / 2) * size;
        int c = compar(key, mid);
        if (c == 0)
            return (void *)mid;
        if (c > 0) {
            lo = mid + size;
            nmemb -= nmemb / 2 + 1;
        } else {
            nmemb /= 2;
        }
    }
    return NULL;
}
