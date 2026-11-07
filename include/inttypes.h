#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

// int64_t is a long, same as glibc and musl
#define PRId8   "d"
#define PRId16  "d"
#define PRId32  "d"
#define PRId64  "ld"
#define PRIi8   "i"
#define PRIi16  "i"
#define PRIi32  "i"
#define PRIi64  "li"
#define PRIu8   "u"
#define PRIu16  "u"
#define PRIu32  "u"
#define PRIu64  "lu"
#define PRIx8   "x"
#define PRIx16  "x"
#define PRIx32  "x"
#define PRIx64  "lx"
#define PRIX8   "X"
#define PRIX16  "X"
#define PRIX32  "X"
#define PRIX64  "lX"
#define PRIo32  "o"
#define PRIo64  "lo"
#define PRIdPTR "ld"
#define PRIuPTR "lu"
#define PRIxPTR "lx"
#define PRIdMAX "ld"
#define PRIuMAX "lu"
#define PRIxMAX "lx"

#define SCNd8   "hhd"
#define SCNd16  "hd"
#define SCNi32  "i"
#define SCNu8   "hhu"
#define SCNu16  "hu"
#define SCNx8   "hhx"
#define SCNx16  "hx"
#define SCNdPTR "ld"
#define SCNuPTR "lu"
#define SCNxPTR "lx"
#define SCNd32  "d"
#define SCNd64  "ld"
#define SCNu32  "u"
#define SCNu64  "lu"
#define SCNx32  "x"
#define SCNx64  "lx"

#define PRIo8   "o"
#define PRIo16  "o"
#define PRIXPTR "lX"
#define PRIiPTR "li"
#define PRIiMAX "li"
#define PRIXMAX "lX"
#define SCNi8   "hhi"
#define SCNi16  "hi"
#define SCNi64  "li"
#define SCNo32  "o"
#define SCNo64  "lo"
#define SCNdMAX "ld"
#define SCNuMAX "lu"
#define SCNxMAX "lx"
#define PRIdFAST8  "d"
#define PRIdFAST16 "d"
#define PRIdFAST32 "d"
#define PRIdFAST64 "ld"
#define PRIuFAST8  "u"
#define PRIuFAST16 "u"
#define PRIuFAST32 "u"
#define PRIuFAST64 "lu"
#define PRIxFAST32 "x"
#define PRIxFAST64 "lx"
#define PRIdLEAST8  "d"
#define PRIdLEAST16 "d"
#define PRIdLEAST32 "d"
#define PRIdLEAST64 "ld"
#define PRIuLEAST8  "u"
#define PRIuLEAST16 "u"
#define PRIuLEAST32 "u"
#define PRIuLEAST64 "lu"

typedef struct {
    intmax_t quot;
    intmax_t rem;
} imaxdiv_t;

intmax_t  imaxabs(intmax_t n);
imaxdiv_t imaxdiv(intmax_t num, intmax_t den);
intmax_t  strtoimax(const char *nptr, char **endptr, int base);
uintmax_t strtoumax(const char *nptr, char **endptr, int base);

#ifdef __cplusplus
}
#endif
