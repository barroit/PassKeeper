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

enum cfg_field_type
{
	FIELD_KDF_ALGORITHM,
	FIELD_HMAC_ALGORITHM,
	FIELD_COMPATIBILITY,
	FIELD_PAGE_SIZE,
	FIELD_KDF_ITER,
	FIELD_KEY,
};

enum key_type
{
	/**
	 * for FIELD_KEY + is_binary_key
	 * order shall not be changed
	 */
	KEY_PASSPHRASE,
	KEY_BINARY,
};

struct field_blob
{
	uint8_t *buf;
	size_t size;
	size_t cap;
};

static void append_field(
	struct field_blob *blob, uint8_t type, const void *data, size_t dlen)
{
	size_t required_size;

	required_size = blob->size + sizeof(uint8_t) + sizeof(size_t) + dlen;
	CAPACITY_GROW(blob->buf, required_size, blob->cap);

	*blob->buf = type;
	blob->size++;

	memcpy(blob->buf + blob->size, &dlen, sizeof(size_t));
	blob->size += sizeof(size_t);

	memcpy(blob->buf + blob->size, data, dlen);
	blob->size += dlen;
}

uint8_t *serialize_cipher_config(
	struct cipher_config *config, const uint8_t *key,
	size_t keylen, bool is_binary_key, size_t *outlen)
{
	/**
	 * [(uint8_t cfg_field_type), (size_t data_length), (data)]
	 */
	struct field_blob *blob = &(struct field_blob){ 0 };
	CAPACITY_GROW(blob->buf, 64, blob->cap);

	if (config->kdf_algorithm &&
	     strcmp(config->kdf_algorithm, CPRDEF_KDF_ALGORITHM))
	{
		append_field(blob, FIELD_KDF_ALGORITHM, config->kdf_algorithm,
				strlen(config->kdf_algorithm));
	}

	if (config->hmac_algorithm &&
	     strcmp(config->hmac_algorithm, CPRDEF_HMAC_ALGORITHM))
	{
		append_field(blob, FIELD_HMAC_ALGORITHM, config->hmac_algorithm,
				strlen(config->hmac_algorithm));
	}

	if (config->cipher_compat != CPRDEF_COMPATIBILITY)
	{
		append_field(blob, FIELD_COMPATIBILITY, &config->cipher_compat,
				sizeof(unsigned));
	}

	if (config->page_size != CPRDEF_PAGE_SIZE)
	{
		append_field(blob, FIELD_PAGE_SIZE, &config->page_size,
				sizeof(unsigned));
	}

	if (config->kdf_iter != CPRDEF_KDF_ITER)
	{
		append_field(blob, FIELD_KDF_ITER, &config->kdf_iter,
				sizeof(unsigned));
	}

	if (key != NULL)
	{
		append_field(blob, FIELD_KEY + is_binary_key, key, keylen);
	}

	CAPACITY_GROW(blob->buf, blob->size + DIGEST_LENGTH, blob->cap);

	*outlen = blob->size;
	return blob->buf;
}
