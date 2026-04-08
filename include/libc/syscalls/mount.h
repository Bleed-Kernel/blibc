#pragma once

int _mount(const char *source,
           const char *target,
           const char *fstype,
           unsigned long flags,
           const void *data);

int _umount(const char *target);
int _umount2(const char *target, int flags);