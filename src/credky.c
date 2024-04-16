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

enum data_type
{
	FIELD_KDF_ALGORITHM,
	FIELD_HMAC_ALGORITHM,
	FIELD_COMPATIBILITY,
	FIELD_PAGE_SIZE,
	FIELD_KDF_ITER,
	FIELD_KEY,
	data_type_elements,
};

enum key_type
{
	/**
	 * for FIELD_KEY + config->is_binary_key
	 * order shall not be changed
	 */
	KEY_PASSPHRASE,
	KEY_BINARY,
};

#define DIGEST_LENGTH 32

#define entry_size(dl) sizeof(uint8_t) + sizeof(size_t) + (dl)

static uint8_t *append_buf(uint8_t *buf, uint8_t type, const void *data, size_t dlen)
{
	*buf++ = type;

	memcpy(buf, &dlen, sizeof(size_t));
	buf += sizeof(size_t);

	if (data && dlen)
	{
		memcpy(buf, data, dlen);
		buf += dlen;
	}

	return buf;
}

uint8_t *serialize_cipher_config(
	struct cipher_config *config, size_t *buflen)
{
	uint8_t *buf0, *buf;
	size_t lenmap[data_type_elements];

	memset(lenmap, 0, sizeof(lenmap));

	if (config->kdf_algorithm && strcmp(config->kdf_algorithm, CIPHER_DEFAULT_KDF_ALGORITHM))
	{
		lenmap[FIELD_KDF_ALGORITHM] = strlen(config->kdf_algorithm);
	}
	else
	{
		/* turn default value into null */
		config->kdf_algorithm = NULL;
	}

	if (config->hmac_algorithm && strcmp(config->hmac_algorithm, CIPHER_DEFAULT_HMAC_ALGORITHM))
	{
		lenmap[FIELD_HMAC_ALGORITHM] = strlen(config->hmac_algorithm);
	}
	else
	{
		config->hmac_algorithm = NULL;
	}

	if (config->cipher_compat != CIPHER_DEFAULT_COMPATIBILITY)
	{
		lenmap[FIELD_COMPATIBILITY] = sizeof(unsigned);
	}

	if (config->page_size != CIPHER_DEFAULT_PAGE_SIZE)
	{
		lenmap[FIELD_PAGE_SIZE] = sizeof(unsigned);
	}

	if (config->kdf_iter != CIPHER_DEFAULT_KDF_ITER)
	{
		lenmap[FIELD_KDF_ITER] = sizeof(unsigned);
	}

	if (config->keylen)
	{
		lenmap[FIELD_KEY] = config->keylen;
	}

	/**
	 * [(uint8_t data_type) + [(uint8_t key_type)], (size_t data_length), (data)]
	 * data shall not contains null-terminator
	 */
	*buflen = 
		entry_size(lenmap[FIELD_KDF_ALGORITHM]) +     /* kdf_algorithm */
		 entry_size(lenmap[FIELD_HMAC_ALGORITHM]) +  /* hmac_algorithm */
		  entry_size(lenmap[FIELD_COMPATIBILITY]) + /* compatibility */
		   entry_size(lenmap[FIELD_PAGE_SIZE]) +   /* page_size */
		    entry_size(lenmap[FIELD_KDF_ITER]) +  /* kdf_iter */
		     entry_size(lenmap[FIELD_KEY]) +     /* key */
		      DIGEST_LENGTH;                    /* buf digest */

	buf0 = xmalloc(*buflen);
	buf = buf0;

	buf = append_buf(buf, FIELD_KDF_ALGORITHM, config->kdf_algorithm, lenmap[FIELD_KDF_ALGORITHM]);
	buf = append_buf(buf, FIELD_HMAC_ALGORITHM, config->hmac_algorithm, lenmap[FIELD_HMAC_ALGORITHM]);
	buf = append_buf(buf, FIELD_COMPATIBILITY, &config->cipher_compat, lenmap[FIELD_COMPATIBILITY]);
	buf = append_buf(buf, FIELD_PAGE_SIZE, &config->page_size, lenmap[FIELD_PAGE_SIZE]);
	buf = append_buf(buf, FIELD_KDF_ITER, &config->kdf_iter, lenmap[FIELD_KDF_ITER]);
	buf = append_buf(buf, FIELD_KEY + config->is_binary_key, config->key, lenmap[FIELD_KEY]);

	return buf0;
}
