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

#ifndef CIPHER_CONFIG_H
#define CIPHER_CONFIG_H

struct cipher_config
{
	char     *kdf_algorithm;
	char     *hmac_algorithm;
	unsigned compatibility;
	unsigned page_size;
	unsigned kdf_iter;
};

struct cipher_key
{
	uint8_t *buf;
	size_t  len;
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

#define CC_INIT						\
	{						\
		.compatibility = CPRDEF_COMPATIBILITY,	\
		.page_size = CPRDEF_PAGE_SIZE,		\
		.kdf_iter = CPRDEF_KDF_ITER,		\
	}

#define CK_INIT { 0 }

#define cc_kdf_algorithm_list\
	TMP_STRARR(CPRDEF_KDF_ALGORITHM, "PBKDF2_HMAC_SHA256", "PBKDF2_HMAC_SHA1", NULL)

#define cc_hmac_algorithm_list\
	TMP_STRARR(CPRDEF_HMAC_ALGORITHM, "HMAC_SHA256", "HMAC_SHA1", NULL)

#define is_cc_kdf_algorithm(algo)\
	findstr(algo, cc_kdf_algorithm_list)

#define is_cc_hmac_algorithm(algo)\
	findstr(algo, cc_hmac_algorithm_list)

#define is_cc_page_size(sz)\
	( in_range_i(sz, CPRMIN_PAGE_SIZE, CPRMAX_PAGE_SIZE) || !is_pow2(sz) )

#define is_cc_compatibility(cap)\
	in_range_i(cap, CPRMIN_COMPATIBILITY, CPRMAX_COMPATIBILITY)

/**
 * this function guarantees the returned buffer
 * has enough space to place the message digest
 */
uint8_t *serialize_cipher_config(const struct cipher_config *config, const struct cipher_key *key, size_t *outlen);

int deserialize_cipher_config(struct cipher_config *config, struct cipher_key *key, const uint8_t *buf, size_t buflen);

/**
 * only use this function on config/key created by
 * deserialize_cipher_config()
 */
void free_cipher_config(struct cipher_config *config, struct cipher_key *key);

/**
 * find cipher config by path, this function does set
 * ‘path’ to NULL if there’s no config found
 */
int find_cipher_config(const char **path);

int resolve_cipher_config(const char *pathname, uint8_t **buf, off_t *len);

char *format_apply_cc_sqlstr(struct cipher_config *cc);

#endif /* CIPHER_CONFIG_H */
