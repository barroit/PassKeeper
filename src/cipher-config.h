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

#define BINKEY_LEN  32
#define HEXKEY_LEN  64
#define KEYSALT_LEN 32

#define HK_STRLEN   (HEXKEY_LEN + 3)
#define SHK_STRLEN  (HK_STRLEN + KEYSALT_LEN)
#define HK_MAXLEN   SHK_STRLEN + 1 /* add one for null-terminator */

#define CC_INIT						\
	{						\
		.compatibility = CPRDEF_COMPATIBILITY,	\
		.page_size = CPRDEF_PAGE_SIZE,		\
		.kdf_iter = CPRDEF_KDF_ITER,		\
	}

#define CK_INIT { 0 }

#define is_binkey_len(l__)\
	( (l__) == HK_STRLEN || (l__) == SHK_STRLEN )

#define is_binkey_wrp(k__, l__)\
	( (k__)[0] == 'x' && (k__)[1] == '\'' && (k__)[(l__) - 1] == '\'' )

#define is_binkey_str(k__, l__)\
	( is_binkey_len(l__) && is_binkey_wrp(k__, l__) )

int check_kdf_algorithm(const char *name);

int check_hmac_algorithm(const char *name);

int check_page_size(unsigned page_size);

int check_compatibility(unsigned compatibility);

/**
 * this function guarantees the returned buffer
 * has enough space to place the message digest
 */
uint8_t *serialize_cipher_config(const struct cipher_config *config, const struct cipher_key *key, size_t *outlen);

void deserialize_cipher_config(struct cipher_config *config, struct cipher_key *key, const uint8_t *buf, size_t buflen);

/**
 * only use this function on config/key created by deserialize_cipher_config()
 */
void free_cipher_config(struct cipher_config *config, struct cipher_key *key);

void resolve_cred_cc_realpath(const char **realpath);

void resolve_cipher_config_file(const char *pathname, uint8_t **buf, off_t *outlen);

char *format_apply_cc_sqlstr(struct cipher_config *cc);

#endif /* CIPHER_CONFIG_H */
