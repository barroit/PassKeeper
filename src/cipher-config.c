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
#include "rawnumop.h"
#include "strbuf.h"
#include "strlist.h"

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

struct cipher_file_blob
{
	uint8_t *buf;
	size_t size;
	size_t cap;
};

#define TYPE_SIZE sizeof(uint8_t)
#define DLEN_SIZE sizeof(size_t)

int resolve_cipher_config_path(const char **pathname)
{
	const char *file;
	struct stat st;

	file = *pathname == NULL ? force_getenv(PK_CRED_KY) : *pathname;

	if (stat(file, &st) != 0)
	{
		goto failure;
	}
	else if (!S_ISREG(st.st_mode))
	{
		warning("Config file at '%s' is not a regular file.", file);
		note("Configuration disabled.");
		goto failure;
	}
	else if (test_file_permission(file, &st, R_OK) != 0)
	{
		warning("Access was denied by config file '%s'", file);
		note("Configuration disabled.");
		goto failure;
	}

	*pathname = file;
	return 0;

failure:
	*pathname = NULL;
	return 1;
}

static const char *kdf_algorithms[] = {
	CPRDEF_KDF_ALGORITHM,
	"PBKDF2_HMAC_SHA256",
	"PBKDF2_HMAC_SHA1",
	NULL,
};

static const char *hmac_algorithms[] = {
	CPRDEF_HMAC_ALGORITHM,
	"HMAC_SHA256",
	"HMAC_SHA1",
	NULL,
};

int check_kdf_algorithm(const char *name)
{
	if (!string_in_array(name, kdf_algorithms))
	{
		return error("'%s' is not found in KDF algorithm "
				"list.", name);
	}

	return 0;
}

int check_hmac_algorithm(const char *name)
{
	if (!string_in_array(name, hmac_algorithms))
	{
		return error("'%s' is not found in HMAC algorithm "
				"list.", name);
	}

	return 0;
}

int check_page_size(unsigned page_size)
{
	if (!in_range_u(page_size, CPRMIN_PAGE_SIZE, CPRMAX_PAGE_SIZE, 1) ||
		!is_pow2(page_size))
	{
		return error("Invalid page size '%u'.", page_size);
	}

	return 0;
}

int check_compatibility(unsigned compatibility)
{
	if (!in_range_u(compatibility, CPRMIN_COMPATIBILITY,
			 CPRMAX_COMPATIBILITY, 1))
	{
		return error("Unknown cipher compatibility '%u'.",
				compatibility);
	}

	return 0;
}

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
	 * [(uint8_t cfg_field_type), (size_t data_length), (data)]
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
	const uint8_t *buf_head, *buf_tail;
	void *fmap[] = {
		&config->kdf_algorithm,
		&config->hmac_algorithm,
		&config->compatibility,
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
