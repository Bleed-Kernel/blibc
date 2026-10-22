#include <unistd.h>
#include <string.h>
#include <stdio.h>

char *optarg;
int   optind = 1, opterr = 1, optopt;

static int next_char = 1;

int getopt(int argc, char *const argv[], const char *optstring) {
    optarg = NULL;

    if (optind == 0) {
        optind    = 1;
        next_char = 1;
    }

    if (optind >= argc || !argv[optind] || argv[optind][0] != '-' || argv[optind][1] == '\0')
        return -1;
    if (strcmp(argv[optind], "--") == 0) {
        optind++;
        return -1;
    }

    const char *arg   = argv[optind];
    int         c     = (unsigned char)arg[next_char++];
    const char *spec  = c != ':' ? strchr(optstring, c) : NULL;
    int         quiet = optstring[0] == ':';

    if (arg[next_char] == '\0') {
        optind++;
        next_char = 1;
    }

    if (!spec || c == '\0') {
        optopt = c;
        if (opterr && !quiet)
            fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
        return '?';
    }

    if (spec[1] == ':') {
        if (next_char > 1) {
            optarg = (char *)arg + next_char;
            optind++;
            next_char = 1;
        } else if (optind < argc) {
            optarg = argv[optind++];
        } else {
            optopt = c;
            if (opterr && !quiet)
                fprintf(stderr, "%s: option -%c needs an argument\n", argv[0], c);
            return quiet ? ':' : '?';
        }
    }
    return c;
}
