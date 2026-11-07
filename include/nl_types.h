#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void *nl_catd;
typedef int   nl_item;

#define NL_SETD         1
#define NL_CAT_LOCALE   1

nl_catd catopen(const char *name, int flag);
char   *catgets(nl_catd cat, int set, int msg, const char *def);
int     catclose(nl_catd cat);

#ifdef __cplusplus
}
#endif
