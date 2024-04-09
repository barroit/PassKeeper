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

#include "parse-options.h"
#include "filesys.h"
#include "rawnumop.h"
#include "credky.h"
#include "strlist.h"
#include "strbuf.h"

static const char *alter_table_sqlstr = 
	"CREATE TABLE account ("
		"id       INTEGER PRIMARY KEY AUTOINCREMENT,"
		"sitename TEXT NOT NULL,"
		"siteurl  TEXT,"
		"username TEXT,"
		"password TEXT,"
		"sqltime  DATETIME NOT NULL,"
		"modtime  DATETIME"
	");"

	"CREATE INDEX idx_sitename ON account(sitename);"

	"CREATE TABLE account_security ("
		"account_id INTEGER PRIMARY KEY,"
		"guard      TEXT,"
		"recovery   TEXT,"
		"FOREIGN KEY (account_id) REFERENCES account(id) ON DELETE CASCADE"
	");"

	"CREATE TABLE account_misc ("
		"account_id INTEGER PRIMARY KEY,"
		"comment    TEXT,"
		"FOREIGN KEY (account_id) REFERENCES account(id) ON DELETE CASCADE"
	");";

static struct
{
	int store_key;
	const char *key;
} user = {
	.store_key = 2,
};

static struct cipher_config cc = {
	.kdf_iter = CIPHER_DEFAULT_KDF_ITER,
	.page_size = CIPHER_DEFAULT_PAGE_SIZE,
	.cipher_compat = CIPHER_DEFAULT_COMPATIBILITY,
};

const char *const cmd_init_usages[] = {
	"pk init [--encrypt[=<key>]] [<options>]",
	NULL,
};

const struct option cmd_init_options[] = {
	OPTION_OPTARG(0, "encrypt", &user.key, 1, "key", "password used to encryption"),
	OPTION_BOOLEAN(0, "remember", &user.store_key, "store password"),
	OPTION_GROUP(""),
	OPTION_STRING(0, "kdf-algorithm", &cc.kdf_algorithm, "KDF algorithm used to generate encryption key for database"),
	OPTION_STRING(0, "hmac-algorithm", &cc.hmac_algorithm, "HMAC algorithm used to detect illegal data tampering"),
	OPTION_UNSIGNED(0, "cipher-compat", &cc.cipher_compat, "version of api to used"),
	OPTION_UNSIGNED(0, "page-size", &cc.page_size, "size of a page"),
	OPTION_UNSIGNED(0, "kdf-iter", &cc.kdf_iter, "key derivation iteration times"),
	OPTION_END(),
};

#define BINKEY_LENGTH		32
#define HEXKEY_LENGTH		64
#define SALT_LENGTH		32
#define HEXKEYSTR_LENGTH	HEXKEY_LENGTH + 3
#define SALT_HEXKEYSTR_LENGTH	HEXKEY_LENGTH + SALT_LENGTH + 3
#define BLOBKEY_MAX		100 /* contains null-terminator */

static const char *kdf_algorithms[] = {
	CIPHER_DEFAULT_KDF_ALGORITHM,
	"PBKDF2HMAC_SHA256",
	"PBKDF2_HMAC_SHA1",
	NULL,
};

static const char *hmac_algorithms[] = {
	CIPHER_DEFAULT_HMAC_ALGORITHM,
	"HMAC_SHA256",
	"HMAC_SHA1",
	NULL,
};

enum key_type
{
	USER_PASSPHRASE = 1,
	PK_BINKEY,
	USER_BINKEY,
};

static inline bool is_binary_key(enum key_type kt)
{
	return kt != USER_PASSPHRASE;
}

#define AF(fn, ...)				\
	do					\
	{					\
		if (fn(__VA_ARGS__))		\
		{				\
			goto init_failure;	\
		}				\
	}					\
	while (0)

static int prepare_file_path(char pathbuf[], const char *prefix, const char *envname, const char *defname)
{
	const char *envpath;
	char *pfpath;

	if ((envpath = getenv(envname)) == NULL)
	{
		pfpath = prefix_filename(prefix, defname);
		strcpy(pathbuf, pfpath);
		free(pfpath);
	}
	else
	{
		strcpy(pathbuf, envpath);
	}

	if (access(pathbuf, F_OK) == 0)
	{
		return 1;
	}

	prepare_file_directory(pathbuf);

	return 0;
}

int cmd_init(UNUSED int argc, const char **argv, const char *prefix)
{
	char db_path[PATH_MAX], ky_path[PATH_MAX];
	bool encrypt_db, use_key_file;
	enum key_type kt;

	char *hxkyst;
	size_t hxkyst_len;

	parse_options(argc, argv, prefix, cmd_init_options, cmd_init_usages, PARSER_ABORT_NON_OPTION);

	if (prepare_file_path(db_path, prefix, PK_CRED_DB, PK_CRED_DB_NM))
	{
		return error("db file '%s' already exists", db_path);
	}

	encrypt_db = !!user.key;
	kt = 0;
	if (!encrypt_db)
	{
		goto init_database;
	}

	kt = (intptr_t)user.key == 1 ? PK_BINKEY :
		*user.key == 'x' ? USER_BINKEY :
		 USER_PASSPHRASE;

	if (kt == USER_PASSPHRASE && *user.key == 0)
	{
		return error("empty passphrase not allowed");
	}

	use_key_file = 0;

	if (cc.kdf_algorithm && is_binary_key(kt))
	{
		if (!string_in_array(cc.kdf_algorithm, kdf_algorithms))
		{
			return error("algorithm '%s' not found", cc.kdf_algorithm);
		}

		use_key_file |= strcmp(cc.kdf_algorithm, CIPHER_DEFAULT_KDF_ALGORITHM);
	}

	if (cc.hmac_algorithm)
	{
		if (!string_in_array(cc.hmac_algorithm, hmac_algorithms))
		{
			return error("algorithm '%s' not found", cc.hmac_algorithm);
		}

		use_key_file |= strcmp(cc.hmac_algorithm, CIPHER_DEFAULT_HMAC_ALGORITHM);
	}

	use_key_file |= cc.kdf_iter != CIPHER_DEFAULT_KDF_ITER && kt == USER_PASSPHRASE;

	if (!in_range_u(cc.page_size, 512, 65536, 1) || !is_pow2(cc.page_size))
	{
		return error("invalid page size '%d'", cc.page_size);
	}
	else
	{
		use_key_file |= cc.page_size != CIPHER_DEFAULT_PAGE_SIZE;
	}

	if (!in_range_u(cc.cipher_compat, CIPHER_MIN_COMPATIBILITY, CIPHER_MAX_COMPATIBILITY, 1))
	{
		return error("unknown cipher compatibility '%d'", cc.cipher_compat);
	}
	else
	{
		use_key_file |= cc.cipher_compat != CIPHER_DEFAULT_COMPATIBILITY;
	}

	use_key_file |= (is_binary_key(kt) && user.store_key) || user.store_key == 1;

	if (use_key_file)
	{
		if (prepare_file_path(ky_path, prefix, PK_CRED_KY, PK_CRED_KY_NM))
		{
			return error("key file '%s' already exists", ky_path);
		}
	}

	switch (kt)
	{
	case PK_BINKEY:;
		char *tmp_hxkyst;

		cc.key = random_bytes(BINKEY_LENGTH);
		cc.keylen = BINKEY_LENGTH;

		tmp_hxkyst = bin2hex(xmemdup(cc.key, cc.keylen), cc.keylen);

		hxkyst = xmalloc(HEXKEYSTR_LENGTH + 1);
		hxkyst_len = snprintf(hxkyst, HEXKEYSTR_LENGTH + 1, "x'%s'", tmp_hxkyst);

		free(tmp_hxkyst);
		break;
	case USER_BINKEY:
		hxkyst_len = strlen(user.key);

		if (hxkyst_len != HEXKEYSTR_LENGTH && hxkyst_len != SALT_HEXKEYSTR_LENGTH)
		{
			return error("invalid key length \"%s\"", user.key);
		}
		else if (!is_hexstr(user.key + 2, HEXKEY_LENGTH))
		{
			return error("key contains invalid char \"%s\"", user.key);
		}
		else if (hxkyst_len == SALT_HEXKEYSTR_LENGTH &&
			!is_saltstr(user.key + 2 + HEXKEY_LENGTH, SALT_LENGTH))
		{
			return error("invalid key salt \"%s\"", user.key);
		}

		cc.key = hex2bin(strdup(user.key + 2), hxkyst_len - 3);
		cc.keylen = (hxkyst_len - 3) / 2;

		hxkyst = strdup(user.key);
		break;
	case USER_PASSPHRASE:
		if ((cc.keylen = strlen(user.key)) == 0)
		{
			return error("passphrase key is empty");
		}

		hxkyst_len = cc.keylen;
		cc.key = xmemdup(user.key, cc.keylen);
		hxkyst = strdup(user.key);
		break;
	}

	cc.is_binary_key = is_binary_key(kt);

	if (!user.store_key)
	{
		if (kt == PK_BINKEY)
		{
			puts(hxkyst);
		}

		free(cc.key);
		cc.key = NULL;
		cc.keylen = 0;
	}

	if (!use_key_file)
	{
		goto init_database;
	}

	uint8_t *cc_buf, *cc_digest;
	size_t cc_buflen, cc_buflen_nodig;
	FILE *fp;

	cc_buf = serialize_cipher_config(&cc, &cc_buflen);

	cc_buflen_nodig = cc_buflen - DIGEST_LENGTH;
	cc_digest = digest_message_sha256(cc_buf, cc_buflen_nodig);

	memcpy(cc_buf + cc_buflen_nodig, cc_digest, DIGEST_LENGTH);
	clean_digest(cc_digest);

	fp = xfopen(ky_path, "wbx");
	xfwrite(cc_buf, sizeof(uint8_t), cc_buflen, fp);

	fclose(fp);
	free(cc_buf);

init_database:;

	struct sqlite3 *db;

	AF(msqlite3_open, db_path, &db);

	if (encrypt_db)
	{
		AF(msqlite3_key, db, hxkyst, hxkyst_len);
	}

	struct strbuf *sb = STRBUF_INIT_P;

	if (cc.kdf_algorithm && is_binary_key(kt))
	{
		strbuf_printf(sb, "PRAGMA cipher_kdf_algorithm = %s;", cc.kdf_algorithm);
	}

	if (cc.hmac_algorithm)
	{
		strbuf_printf(sb, "PRAGMA cipher_hmac_algorithm = %s;", cc.hmac_algorithm);
	}

	if (cc.kdf_iter != CIPHER_DEFAULT_KDF_ITER && is_binary_key(kt))
	{
		strbuf_printf(sb, "PRAGMA kdf_iter = %d;", cc.kdf_iter);
	}

	if (cc.page_size != CIPHER_DEFAULT_PAGE_SIZE)
	{
		strbuf_printf(sb, "PRAGMA cipher_page_size = %d;", cc.page_size);
	}

	/* always specify this */
	strbuf_printf(sb, "PRAGMA cipher_compatibility = %d;", cc.cipher_compat);

	AF(msqlite3_exec, db, sb->buf);

	AF(msqlite3_exec, db, alter_table_sqlstr);

	sqlite3_close(db);
	return 0;

init_failure:

	sqlite3_close(db);

	unlink(db_path);
	unlink(ky_path);

	return 2;
}