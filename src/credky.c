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
#include "rawnumop.h"

enum cfg_field_type
{
	FIELD_KDF_ALGORITHM,
	FIELD_HMAC_ALGORITHM,
	FIELD_COMPATIBILITY,
	FIELD_PAGE_SIZE,
	FIELD_KDF_ITER,
	FIELD_KEY,
	cfg_field_type_count,
};

enum key_type
{
	/**
	 * for FIELD_KEY + is_binary_key
	 * order shall not be changed
	 */
	KEY_PASSPHRASE,
	KEY_BINARY,
	key_type_count,
};

struct field_blob
{
	uint8_t *buf;
	size_t size;
	size_t cap;
};

#define TYPE_SIZE sizeof(uint8_t)
#define DLEN_SIZE sizeof(size_t)

static void append_field(
	struct field_blob *blob, uint8_t type, const void *data, size_t dlen)
{
	size_t required_size;

	required_size = blob->size + TYPE_SIZE + DLEN_SIZE + dlen;
	CAPACITY_GROW(blob->buf, required_size, blob->cap);

	*blob->buf = type;
	blob->size += TYPE_SIZE;

	memcpy(blob->buf + blob->size, &dlen, DLEN_SIZE);
	blob->size += DLEN_SIZE;

	memcpy(blob->buf + blob->size, data, dlen);
	blob->size += dlen;
}

uint8_t *serialize_cipher_config(
	const struct cipher_config *config,
	const struct cipher_key *key, size_t *outlen)
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

	if (key->buf != NULL)
	{
		append_field(blob, FIELD_KEY + key->is_binary,
				key->buf, key->size);
	}

	CAPACITY_GROW(blob->buf, blob->size + CIPHER_DIGEST_LENGTH, blob->cap);

	*outlen = blob->size;
	return blob->buf;
}

#define SAFE_INCREMENT(buf0__, len0__, buf__, len__)		\
	do							\
	{							\
		if (buf__ + len__ > buf0__ + len0__)		\
		{						\
			bug("add %"PRIuMAX" to buf is beyond "	\
				"the buf size", len__);		\
		}						\
								\
		buf__ += len__;					\
	}							\
	while (0)

void deserialize_cipher_config(
	struct cipher_config *config, struct cipher_key *key,
	const uint8_t *buf, size_t buflen)
{
	memset(config, 0, sizeof(struct cipher_config));
	memset(key, 0, sizeof(struct cipher_key));

	const uint8_t *buf_head, *buf_tail;
	void *fmap[] = {
		&config->kdf_algorithm,
		&config->hmac_algorithm,
		&config->cipher_compat,
		&config->page_size,
		&config->kdf_iter,
		&key->buf,
	};

	buf_head = buf;
	buf_tail = buf + buflen;

	while (39)
	{

	enum cfg_field_type type;
	size_t dlen;

	type = *buf;
	if (!in_range(type, 0, cfg_field_type_count + key_type_count, false))
	{
		bug("field type value shall not be %d", type);
	}
	SAFE_INCREMENT(buf_head, buflen, buf, TYPE_SIZE);

	memcpy(&dlen, buf, DLEN_SIZE);
	SAFE_INCREMENT(buf_head, buflen, buf, DLEN_SIZE);

	switch (type)
	{
	case FIELD_KDF_ALGORITHM:
	case FIELD_HMAC_ALGORITHM:
		*(char **)fmap[type] = xmalloc(dlen);

		memcpy(*(char **)fmap[type], buf, dlen);

		break;
	case FIELD_COMPATIBILITY:
	case FIELD_PAGE_SIZE:
	case FIELD_KDF_ITER:
		memcpy(fmap[type], buf, dlen);

		break;
	case FIELD_KEY + KEY_PASSPHRASE:
	case FIELD_KEY + KEY_BINARY:
		*(uint8_t **)fmap[FIELD_KEY] = xmalloc(dlen);

		memcpy(*(uint8_t **)fmap[FIELD_KEY], buf, dlen);
		key->size = dlen;
		key->is_binary = type - FIELD_KEY == KEY_BINARY;

		break;
	default:
		bug("field type value shall not be %d", type);
	}
	SAFE_INCREMENT(buf_head, buflen, buf, dlen);

	/* buf must be less than buf tail */
	if (buf > buf_tail)
	{
		bug("buf is beyond buf tail");
	}
	else if (buf == buf_tail)
	{
		break;
	}

	}
}

void free_cipher_config(struct cipher_config *config, struct cipher_key *key)
{
	void *ptrlist[] = {
		config->kdf_algorithm,
		config->hmac_algorithm,
		key->buf,
		NULL,
	}, **iter;

	iter = ptrlist;
	while (*iter != NULL)
	{
		free(*iter++);
	}
}
