/****************************************************************************
**
** Copyright 2023, 2024 Jiamu Sun
** Contact: barroit@linux.com
**
** This file is part of PassKeeper.
**
** PassKeeper is free software: you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation, either version 3 of the License, or (at your
** option) any later version.
**
** PassKeeper is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with PassKeeper. If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef SECURITY_H
#define SECURITY_H

int random_bytes_routine(uint8_t **buf, size_t len, bool alloc_mem);

#define random_bytes(buf__, len__) random_bytes_routine(buf__, len__, false)
#define random_bytes_alloc(buf__, len__) random_bytes_routine(buf__, len__, true)

/**
 * convert at most `hexsz` characters of the `hex` string to
 * binary data, `size` must be a multiple of 2, this operation
 * is in-place and `hex` shall be modified, returned pointer
 * is the same as `hex`
 */
uint8_t *hex2bin(char *hex, size_t hexsz);

/**
 * Convert at most `binsz` bytes of the `bin` binary data to
 * hex characters, and making it null-terminated.
 * 
 * The pointer points to `bin` shall be invalid after convert.
 * 
 * Returned value shall be freed by caller.
 */
char *bin2hex(uint8_t *bin, size_t binsz);

/**
 * same as the `bin2hex` except it convert binary data to
 * blob string (hex key is wrapped by x'')
 */
char *bin2blob(uint8_t *binkey, size_t binlen);

/**
 * check if at most `size` characters of the `hex` string are
 * valid hex characters
 */
bool is_hexstr(const char *hex, size_t size);

/**
 * check if at most `size` characters of the `salt` string are
 * valid hex characters
 */
bool is_saltstr(const char *salt, size_t size);

uint8_t *digest_message_sha256(const uint8_t *message, size_t message_length);

#define clean_digest(addr__) OPENSSL_free(addr__)

int verify_digest_sha256(const uint8_t *message, size_t message_length, const uint8_t *prev_digest);

void secure_destroy(void *ptr, size_t len);

size_t read_cmdkey(char **buf0);

#endif /* SECURITY_H */
