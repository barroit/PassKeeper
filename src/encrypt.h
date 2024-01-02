#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <stddef.h>

void *get_database_key(const char *db_key_pathname, size_t *size);

#include "os.h"

#ifdef PK_USE_ARC4RANDOM

char itoh(unsigned char c);

void *genbytes(size_t length); /* length in bytes */

char *btoh(void *data, size_t length); /* length of bytes data */

#endif /* PK_USE_ARC4RANDOM */

void *htob(const char *data, size_t *size); /* size after convert */

unsigned char htoi(char c);

#endif