#pragma once

#define IF_NAMESIZE 16
#define IFNAMSIZ    IF_NAMESIZE

// theres no network stack, no interface ever exists
unsigned int if_nametoindex(const char *name);
char *if_indextoname(unsigned int index, char *name);
