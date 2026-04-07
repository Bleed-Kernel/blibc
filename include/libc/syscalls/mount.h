#pragma once

int _mount(const char *source,
           const char *target,
           const char *fstype,
           unsigned long flags,
           const void *data);