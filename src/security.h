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

#define BINKEY_LEN 32
#define HEXKEY_LEN 64

int random_bytes_routine(uint8_t **buf, size_t len, bool alloc_mem);

#define random_bytes(buf__, len__) random_bytes_routine(buf__, len__, true)
#define random_bytes_buffered(buf__, len__) random_bytes_routine(buf__, len__, false)

/**
 * convert at most hex_len characters of the hex0 string to binary data, 
 * hex_len must be a multiple of 2, this operation is in-place and hex0
 * shall be modified
 * 
 * NOTE: pointer assigend to *out is the same as hex0
 */
size_t hex2bin(uint8_t **out, char *hex0, size_t hex_len);

/**
 * convert at most bin_len bytes of the bin0 to hex characters, and making
 * it null-terminated, pointer points to bin0 shall be invalid after convert
 */
size_t bin2hex(char **out, uint8_t *bin0, size_t bin_len);

/**
 * same as the bin2hex except it convert binary data to blob string (hex
 * key is wrapped by x'')
 */
size_t bin2blob(char **out, uint8_t *bin_key, size_t bin_len);

/**
 * same as the hex2bin except it convert blob string (hex key is wrapped
 * by x'') to binary data
 */
size_t blob2bin(uint8_t **out, char *blob, size_t blob_len);

bool is_blob_key(const char *key, size_t len);

uint8_t *digest_message_sha256(const uint8_t *message, size_t message_length);

#define clean_digest(addr__) OPENSSL_free(addr__)

int verify_digest_sha256(const uint8_t *message, size_t message_length, const uint8_t *prev_digest);

int termios_disable_echo(struct termios *term0);

int termios_restore_config(struct termios *term0);

size_t read_cmdkey(char **key0, const char *message);

#endif /* SECURITY_H */
