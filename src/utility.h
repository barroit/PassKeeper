#ifndef UTILITY_H
#define UTILITY_H

#include <stddef.h>
#include <stdbool.h>

#define STRINGIFY(str) ((str) == ((void *)0) ? "(null)" : (*str) == '\0' ? "(empty)" : (str))

#define STRBOOL(v) ((v) ? "true" : "false")

#define MAX(x, y) ((x > y) ? x : y)

#define MIN(x, y) ((x < y) ? x : y)

#define IN_RANGE(p, x1, x2) ((p >= x1) && (p <= x2))

bool is_positive_integer(const char *str);

bool exists(const char *pathname);

bool is_rw_file(const char *pathname);

bool is_rwx_dir(const char *dirname);

bool is_empty_string(const char *string);

bool is_hexchr(char c);

char *strpad(size_t padlen);

char *strapd(const char *origin, const char *append);

char *strsub(const char *src, size_t start, size_t cpylen);

char *prefix(const char *pathname);

#endif /* UTILITY_H */