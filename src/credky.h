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

#ifndef CREDKY_H
#define CREDKY_H

struct cipher_config
{
	char     *kdf_algorithm;
	char     *hmac_algorithm;
	unsigned cipher_compat;
	unsigned page_size;
	unsigned kdf_iter;
};

struct cipher_key
{
	uint8_t *buf;
	size_t  size;
	bool    is_binary;
};

/* CPRDEF stands for cipher default */
#define CPRDEF_KDF_ALGORITHM  "PBKDF2_HMAC_SHA512"
#define CPRDEF_HMAC_ALGORITHM "HMAC_SHA512"
#define CPRDEF_COMPATIBILITY  4
#define CPRDEF_PAGE_SIZE      4096
#define CPRDEF_KDF_ITER       256000

#define CPRMIN_COMPATIBILITY 1
#define CPRMAX_COMPATIBILITY CPRDEF_COMPATIBILITY

#define CPRMIN_PAGE_SIZE 512
#define CPRMAX_PAGE_SIZE 65536

#define CIPHER_DIGEST_LENGTH 32

#define BINKEY_LEN  32
#define HEXKEY_LEN  64
#define KEYSALT_LEN 32

#define HK_STRLEN   (HEXKEY_LEN + 3)
#define SHK_STRLEN  (HK_STRLEN + KEYSALT_LEN)
#define HK_MAXLEN   SHK_STRLEN + 1 /* add one for null-terminator */

static inline FORCEINLINE bool is_binkey_len(size_t len)
{
	return len == HK_STRLEN || len == SHK_STRLEN;
}

static inline FORCEINLINE bool is_binkey_wrp(const char *key, size_t len)
{
	return key[0] == 'x' &&
		key[1] == '\'' &&
		 key[len - 1] == '\'';
}

static inline FORCEINLINE bool is_binkey_str(const char *key, size_t len)
{
	return is_binkey_len(len) && is_binkey_wrp(key, len);
}

/**
 * this function guarantees the returned buffer
 * has enough space to place the message digest
 */
uint8_t *serialize_cipher_config(const struct cipher_config *config, const struct cipher_key *key, size_t *outlen);

void deserialize_cipher_config(struct cipher_config *config, struct cipher_key *key, const uint8_t *buf, size_t buflen);

void free_cipher_config(struct cipher_config *config, struct cipher_key *key);

#endif /* CREDKY_H */
