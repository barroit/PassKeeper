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

#include "credky.h"

enum cipher_config_field
{
	FIELD_KDF_ALGORITHM = 1,
	FIELD_HMAC_ALGORITHM,
	FIELD_COMPATIBILITY,
	FIELD_PAGE_SIZE,
	FIELD_KDF_ITER,
	FIELD_KEY,
};

enum key_type
{
	KEY_PASSPHRASE = 1,
	KEY_BINARY,
};

#define LENGTH(m) sizeof(uint8_t) + sizeof(size_t) + m

#define APPEND(o, p, l)			\
	do				\
	{				\
		memcpy(o, p, l);	\
		o += l;			\
	}				\
	while (0)

uint8_t *serialize_cipher_config(
	const struct cipher_config *config, size_t *out_buflen)
{
	uint8_t *out, *out_ptr;
	size_t config_kdf_algorithm_memlen, config_hmac_algorithm_memlen;

	config_kdf_algorithm_memlen  = strlen(config->kdf_algorithm) + 1;
	config_hmac_algorithm_memlen = strlen(config->hmac_algorithm) + 1;

	// void *config_field = {
	// 	FIELD_KDF_ALGORITHM, config->kdf_algorithm, 
	// }

	*out_buflen =
		LENGTH(config_kdf_algorithm_memlen) +
		 LENGTH(config_hmac_algorithm_memlen) +
		  LENGTH(sizeof(unsigned)) +
		   LENGTH(sizeof(unsigned)) +
		    LENGTH(sizeof(unsigned)) +
		     LENGTH(config->keylen);

	out = xmalloc(*out_buflen);
	out_ptr = out;

	APPEND(out_ptr, config->kdf_algorithm, config_kdf_algorithm_memlen);
	APPEND(out_ptr, config->hmac_algorithm, config_hmac_algorithm_memlen);
	APPEND(out_ptr, &config->compatibility, sizeof(unsigned));
	APPEND(out_ptr, &config->page_size, sizeof(unsigned));
	APPEND(out_ptr, &config->kdf_iter, sizeof(unsigned));
	APPEND(out_ptr, config->key, config->keylen);

	return out;
}