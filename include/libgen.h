#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// posix versions, they can change path
char *basename(char *path);
char *dirname(char *path);

#ifdef __cplusplus
}
#endif
