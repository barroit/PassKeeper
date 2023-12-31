#ifndef UTILITY_H
#define UTILITY_H

#include <stddef.h>
#include <stdbool.h>

#include "os.h"

#define STRINGIFY(str) ((str) == ((void *)0) ? "(null)" : (*str) == '\0' ? "(empty)" : (str))

#define STRBOOL(v) ((v) ? "true" : "false")

#define MAX(x, y) ((x > y) ? x : y)

#define MIN(x, y) ((x < y) ? x : y)

bool is_positive_integer(const char *str);

bool is_rwx_dir(const char *dirname);

bool is_rw_file(const char *pathname);

bool is_empty_string(const char *string);

char *strpad(size_t padlen);

char *dirof(const char *pathname);

char *strapd(const char *origin, const char *append);

char *strsub(const char *src, size_t start, size_t cpylen);

int dirmake(const char *pathname);

#ifdef PK_USE_ARC4RANDOM

/* generate `length` bytes data using arc4random */
void *genbytes(size_t length); /* length in bytes */

/* name stands for "bytes to hexadecimal characters". */
char *btoh(void *data, size_t length); /* length of bytes data */

#endif /* PK_USE_ARC4RANDOM */

#endif /* UTILITY_H */