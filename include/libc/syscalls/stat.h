#pragma once

struct stat;

long _stat(const char *path, struct stat *st);
