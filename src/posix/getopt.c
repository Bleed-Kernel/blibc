#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>

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

static int getopt_long_any(int argc, char *const *argv, const char *optstring,
                           const struct option *longopts, int *idx, int only) {
    if (optind == 0) {
        optind    = 1;
        next_char = 1;
    }
    if (optind >= argc || !argv[optind] || next_char != 1)
        return getopt(argc, argv, optstring);

    const char *arg = argv[optind];
    const char *name = NULL;
    if (arg[0] == '-' && arg[1] == '-' && arg[2])
        name = arg + 2;
    else if (only && arg[0] == '-' && arg[1] && arg[1] != '-')
        name = arg + 1;
    if (!name)
        return getopt(argc, argv, optstring);

    size_t len = strcspn(name, "=");
    const struct option *hit = NULL;
    int hits = 0;
    for (const struct option *o = longopts; o && o->name; o++) {
        if (strncmp(o->name, name, len) != 0)
            continue;
        if (strlen(o->name) == len) {
            hit = o;
            hits = 1;
            break;
        }
        hit = o;
        hits++;
    }
    if (!hit && only)
        return getopt(argc, argv, optstring);

    int quiet = optstring[0] == ':';
    optind++;
    optarg = NULL;
    if (hits != 1) {
        optopt = 0;
        if (opterr && !quiet)
            fprintf(stderr, "%s: %s option '%s'\n", argv[0], hits ? "ambiguous" : "unknown", arg);
        return '?';
    }
    if (idx)
        *idx = (int)(hit - longopts);

    if (name[len] == '=') {
        if (hit->has_arg == no_argument) {
            optopt = hit->val;
            if (opterr && !quiet)
                fprintf(stderr, "%s: option '--%s' takes no argument\n", argv[0], hit->name);
            return '?';
        }
        optarg = (char *)name + len + 1;
    } else if (hit->has_arg == required_argument) {
        if (optind >= argc) {
            optopt = hit->val;
            if (opterr && !quiet)
                fprintf(stderr, "%s: option '--%s' needs an argument\n", argv[0], hit->name);
            return quiet ? ':' : '?';
        }
        optarg = argv[optind++];
    }

    if (hit->flag) {
        *hit->flag = hit->val;
        return 0;
    }
    return hit->val;
}

int getopt_long(int argc, char *const *argv, const char *optstring, const struct option *longopts, int *idx) {
    return getopt_long_any(argc, argv, optstring, longopts, idx, 0);
}

int getopt_long_only(int argc, char *const *argv, const char *optstring, const struct option *longopts, int *idx) {
    return getopt_long_any(argc, argv, optstring, longopts, idx, 1);
}
