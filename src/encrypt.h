#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <stddef.h>

void *get_binary_key(size_t length); /* length in bytes */

char *bin_to_hex(void *data, size_t length); /* length of bytes data */

void *hex_to_bin(const char *data, size_t *size); /* size after convert */

char byte_to_hexchar(unsigned char c);

unsigned char hexchar_to_byte(char c);

#endif