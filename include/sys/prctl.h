#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define PR_SET_PDEATHSIG        1
#define PR_GET_PDEATHSIG        2
#define PR_GET_DUMPABLE         3
#define PR_SET_DUMPABLE         4
#define PR_GET_KEEPCAPS         7
#define PR_SET_KEEPCAPS         8
#define PR_GET_TIMING           13
#define PR_SET_TIMING           14
#define PR_SET_NAME             15
#define PR_GET_NAME             16
#define PR_GET_SECCOMP          21
#define PR_SET_SECCOMP          22
#define PR_CAPBSET_READ         23
#define PR_CAPBSET_DROP         24
#define PR_GET_TSC              25
#define PR_SET_TSC              26
#define PR_SET_TIMERSLACK       29
#define PR_GET_TIMERSLACK       30
#define PR_SET_PTRACER          0x59616d61
#define PR_SET_PTRACER_ANY      (-1UL)
#define PR_SET_CHILD_SUBREAPER  36
#define PR_GET_CHILD_SUBREAPER  37
#define PR_SET_NO_NEW_PRIVS     38
#define PR_GET_NO_NEW_PRIVS     39
#define PR_GET_TID_ADDRESS      40
#define PR_SET_THP_DISABLE      41
#define PR_GET_THP_DISABLE      42
#define PR_SET_VMA              0x53564d41
#define PR_SET_VMA_ANON_NAME    0
#define PR_SET_MM               35

// names are kept per thread, the rest says it worked or einval
int prctl(int option, ...);

#ifdef __cplusplus
}
#endif
