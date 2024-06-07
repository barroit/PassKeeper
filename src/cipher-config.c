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

#include "cipher-config.h"
#include "security.h"
#include "strbuf.h"
#include "strlist.h"
#include "filesys.h"

enum field_type
{
	field_type_start,
	FIELD_KDF_ALGORITHM,
	FIELD_HMAC_ALGORITHM,
	FIELD_COMPATIBILITY,
	FIELD_PAGE_SIZE,
	FIELD_KDF_ITER,
	FIELD_KEY,

	/**
	 * for FIELD_KEY + is_binary_key
	 * order shall not be changed
	 */
	FIELD_KEY_PASSPHRASE,
	FIELD_KEY_BINARY,
	field_type_end,
};

struct cipher_file_blob
{
	uint8_t *buf;
	size_t size;
	size_t cap;
};

#define TYPE_SIZE sizeof(uint8_t)
#define DLEN_SIZE sizeof(size_t)

static void append_field(
	struct cipher_file_blob *blob, uint8_t type,
	const void *data, size_t dlen)
{
	size_t required_size;

	required_size = blob->size + TYPE_SIZE + DLEN_SIZE + dlen;
	CAPACITY_GROW(blob->buf, required_size, blob->cap);

	memcpy(blob->buf + blob->size, &type, TYPE_SIZE);
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
	 * [(uint8_t field_type), (size_t data_length), (data)]
	 */
	struct cipher_file_blob *blob = &(struct cipher_file_blob){ 0 };
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

	if (config->compatibility != CPRDEF_COMPATIBILITY)
	{
		append_field(blob, FIELD_COMPATIBILITY, &config->compatibility,
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
				key->buf, key->len);
	}

	CAPACITY_GROW(blob->buf, blob->size + CIPHER_DIGEST_LENGTH, blob->cap);

	*outlen = blob->size;
	return blob->buf;
}

#define ST_BUF_INC(buf0__, len0__, buf__, len__)		\
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

int deserialize_cipher_config(
	struct cipher_config *config, struct cipher_key *key,
	const uint8_t *buf, size_t len)
{
	const uint8_t *buf0, *buf9;
	void *fmap[] = {
		&config->kdf_algorithm,
		&config->hmac_algorithm,
		&config->compatibility,
		&config->page_size,
		&config->kdf_iter,
		&key->buf,
	};

	buf0 = buf;
	buf9 = buf + len;

	while (39)
	{
/* START LOOP */

	enum field_type type;
	size_t dtlen;

	type = *buf;
	if (!in_range_e(type, field_type_start, field_type_end))
	{
		errno = EINCPHR;
		return 1;
	}
	ST_BUF_INC(buf0, len, buf, TYPE_SIZE);

	memcpy(&dtlen, buf, DLEN_SIZE);
	ST_BUF_INC(buf0, len, buf, DLEN_SIZE);

	switch (type)
	{
	case FIELD_KDF_ALGORITHM:
	case FIELD_HMAC_ALGORITHM:
		*(char **)fmap[type] = xmalloc(dtlen);

		memcpy(*(char **)fmap[type], buf, dtlen);

		break;
	case FIELD_COMPATIBILITY:
	case FIELD_PAGE_SIZE:
	case FIELD_KDF_ITER:
		memcpy(fmap[type], buf, dtlen);

		break;
	case FIELD_KEY_PASSPHRASE:
	case FIELD_KEY_BINARY:
		*(uint8_t **)fmap[FIELD_KEY] = xmalloc(dtlen);

		memcpy(*(uint8_t **)fmap[FIELD_KEY], buf, dtlen);
		key->len = dtlen;
		key->is_binary = type == FIELD_KEY_BINARY;

		break;
	default:
		bug("field type shall not be %d", type);
	}
	ST_BUF_INC(buf0, len, buf, dtlen);

	/* buf must <= buf tail */
	if (buf > buf9)
	{
		errno = EINCPHR;
		return 1;
	}
	else if (buf == buf9)
	{
		break;
	}

/* END LOOP */
	}

	return 0;
}

void free_cipher_config(struct cipher_config *cc, struct cipher_key *ck)
{
	free(cc->kdf_algorithm);
	free(cc->hmac_algorithm);
	sfree(ck->buf, ck->len);
}

int find_cipher_config(const char **path)
{
	if (access_regular(*path, R_OK) != 0)
	{
		if (errno != ENOENT)
		{
			/* errno already set */
			return 1;
		}
		else
		{
			*path = NULL;
		}
	}

	return 0;
}

int resolve_cipher_config(
	const char *pathname, uint8_t **buf1, off_t *len1)
{
	int fd;
	uint8_t *buf;
	off_t len;

	xiopath = pathname;
	fd = xopen(pathname, O_RDONLY);

	if ((len = xlseek(fd, 0, SEEK_END)) < CIPHER_DIGEST_LENGTH)
	{
		/* too small */
		errno = EINCPHR;
		return 1;
	}

	xlseek(fd, 0, SEEK_SET);

	buf = xmalloc(len);
	xread(fd, buf, len);

	close(fd);

	len -= CIPHER_DIGEST_LENGTH;
	if (verify_digest_sha256(buf, len, buf + len) != 0)
	{
		errno = EINCPHR;
		return 1;
	}

	*buf1 = buf;
	*len1 = len;

	return 0;
}

char *format_apply_cc_sqlstr(struct cipher_config *cc)
{
	struct strbuf *sb = STRBUF_INIT_PTR;

	if (cc->kdf_algorithm != NULL)
	{
		strbuf_printf(sb, "PRAGMA cipher_kdf_algorithm = %s;",
				cc->kdf_algorithm);
	}

	if (cc->hmac_algorithm != NULL)
	{
		strbuf_printf(sb, "PRAGMA cipher_hmac_algorithm = %s;",
				cc->hmac_algorithm);
	}

	if (cc->kdf_iter != CPRDEF_KDF_ITER)
	{
		strbuf_printf(sb, "PRAGMA kdf_iter = %d;", cc->kdf_iter);
	}

	if (cc->page_size != CPRDEF_PAGE_SIZE)
	{
		strbuf_printf(sb, "PRAGMA cipher_page_size = %d;",
				cc->page_size);
	}

	if (cc->compatibility != CPRDEF_COMPATIBILITY)
	{
		strbuf_printf(sb, "PRAGMA cipher_compatibility = %d;",
				cc->compatibility);
	}

	return sb->capacity == 0 ? NULL : sb->buf;
}
