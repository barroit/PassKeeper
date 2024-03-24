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

static const char *create_table_sql =
	"CREATE TABLE account ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"sitename TEXT NOT NULL,"
		"siteurl TEXT,"
		"username TEXT,"
		"password TEXT,"
		"authtext TEXT,"
		"bakcode TEXT,"
		"comment TEXT,"
		"sqltime DATETIME DEFAULT (datetime('now', '+9 hours')),"
		"modtime DATETIME"
	");"
	"CREATE TRIGGER update_modtime "
	"AFTER UPDATE ON account "
	"FOR EACH ROW "
	"BEGIN "
		"UPDATE account SET modtime = datetime('now', '+9 hours') WHERE id = old.id;"
	"END;";

static struct
{
	int store_key;
	const char *key;
} user;

static struct cipher_config cc = {
	.kdf_iter = CIPHER_DEFAULT_KDF_ITER,
	.page_size = CIPHER_DEFAULT_PAGE_SIZE,
	.compatibility = CIPHER_DEFAULT_COMPATIBILITY,
};

const char *const cmd_init_usages[] = {
	"pk init [--encrypt[=<pwd>]] [<options>]",
	NULL,
};

const struct option cmd_init_options[] = {
	OPTION_OPTARG(0, "encrypt", &user.key, 1, "pwd", "password used to encryption"),
	OPTION_BOOLEAN_F(0, "remember", &user.store_key, "store password", 0),
	OPTION_GROUP(""),
	OPTION_STRING(0, "kdf-algorithm", &cc.kdf_algorithm, "KDF algorithm used to generate encryption key for database"),
	OPTION_STRING(0, "hmac-algorithm", &cc.hmac_algorithm, "HMAC algorithm used to detect illegal data tampering"),
	OPTION_UNSIGNED(0, "compatibility", &cc.compatibility, "version of api to used"),
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

static void prepare_file_path(char pathbuf[], const char *prefix, const char *envname, const char *defname)
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
		die("file '%s' already exists", pathbuf);
	}

	prepare_file_directory(pathbuf);
}

enum key_type
{
	PK_BINKEY = 1,
	USER_BINKEY,
	USER_PASSPHRASE,
};

static enum key_type get_key_type(void)
{
	if (!user.key)
	{
		return 0;
	}
	else if ((intptr_t)user.key == 1)
	{
		return PK_BINKEY;
	}
	else if (*user.key == 'x')
	{
		return USER_BINKEY;
	}
	else
	{
		return USER_PASSPHRASE;
	}
}

#define af(fn, ...)				\
	do					\
	{					\
		if (fn(__VA_ARGS__))		\
		{				\
			goto init_failure;	\
		}				\
	}					\
	while (0)


int cmd_init(UNUSED int argc, const char **argv, const char *prefix)
{
	char db_path[PATH_MAX], ky_path[PATH_MAX];
	bool encrypt_db, use_key_file;
	enum key_type kt;

	char *hxkyst;
	size_t hxkyst_len;

	parse_options(argc, argv, prefix, cmd_init_options, cmd_init_usages, PARSER_ABORT_NON_OPTION);

	encrypt_db = !!user.key;
	kt = get_key_type();
	use_key_file = 0;

	if (user.store_key && !encrypt_db)
	{
		return error("remember option must be used together with encrypt option");
	}

	/* check kdf algorithm */
	if (cc.kdf_algorithm)
	{
		if (!in_string_array(kdf_algorithms, cc.kdf_algorithm))
		{
			return error("algorithm '%s' not found", cc.kdf_algorithm);
		}

		use_key_file |= strcmp(cc.kdf_algorithm, CIPHER_DEFAULT_KDF_ALGORITHM);
	}

	/* check hmac algorithm */
	if (cc.hmac_algorithm)
	{
		if (!in_string_array(hmac_algorithms, cc.hmac_algorithm))
		{
			return error("algorithm '%s' not found", cc.hmac_algorithm);
		}

		use_key_file |= strcmp(cc.hmac_algorithm, CIPHER_DEFAULT_HMAC_ALGORITHM);
	}

	/* check key iter */
	use_key_file |= cc.kdf_iter != CIPHER_DEFAULT_KDF_ITER;

	/* check page size */
	if (!in_range_u(cc.page_size, 512, 65536, 1) || !is_pow2(cc.page_size))
	{
		return error("invalid page size '%d'", cc.page_size);
	}
	else
	{
		use_key_file |= cc.page_size != CIPHER_DEFAULT_PAGE_SIZE;
	}

	/* check compatibility */
	if (!in_range_u(cc.compatibility, CIPHER_MIN_COMPATIBILITY, CIPHER_MAX_COMPATIBILITY, 1))
	{
		return error("unknown compatibility '%d'", cc.compatibility);
	}
	else
	{
		use_key_file |= cc.compatibility != CIPHER_DEFAULT_COMPATIBILITY;
	}

	/* check if store key */
	use_key_file |= 
		kt == PK_BINKEY ||
		 kt == USER_BINKEY ||
		  (kt == USER_PASSPHRASE && user.store_key);

	prepare_file_path(db_path, prefix, PK_CRED_DB, PK_CRED_DB_NM);

	if (use_key_file)
	{
		prepare_file_path(ky_path, prefix, PK_CRED_KY, PK_CRED_KY_NM);
	}

	if (!encrypt_db)
	{
		goto setup_key_file;
	}

	switch (kt)
	{
	case PK_BINKEY:
		char *tmp_hxkyst;

		cc.key = random_bytes(BINKEY_LENGTH);
		cc.keylen = BINKEY_LENGTH;

		tmp_hxkyst = bin2hex(xmemdup(cc.key, cc.keylen), cc.keylen);

		hxkyst = xmalloc(HEXKEYSTR_LENGTH);
		hxkyst_len = snprintf(hxkyst, HEXKEYSTR_LENGTH, "x'%s'", tmp_hxkyst);

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
			exit(error("passphrase key is empty"));
		}

		hxkyst_len = cc.keylen;
		cc.key = xmemdup(user.key, cc.keylen);
		hxkyst = strdup(user.key);
		break;
	}

	cc.is_binary_key = kt == USER_BINKEY || kt == PK_BINKEY;

setup_key_file:

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

init_database:

	struct sqlite3 *db;

	af(msqlite3_open, db_path, &db);

	if (encrypt_db)
	{
		af(msqlite3_key, db, hxkyst, hxkyst_len);
	}

	struct strbuf *sb = STRBUF_INIT_P;

	if (cc.kdf_algorithm)
	{
		strbuf_printf(sb, "PRAGMA cipher_kdf_algorithm = %s;", cc.kdf_algorithm);
	}

	if (cc.hmac_algorithm)
	{
		strbuf_printf(sb, "PRAGMA cipher_hmac_algorithm = %s;", cc.hmac_algorithm);
	}

	if (cc.kdf_iter != CIPHER_DEFAULT_KDF_ITER)
	{
		strbuf_printf(sb, "PRAGMA kdf_iter = %d;", cc.kdf_iter);
	}

	if (cc.page_size != CIPHER_DEFAULT_PAGE_SIZE)
	{
		strbuf_printf(sb, "PRAGMA cipher_page_size = %d;", cc.page_size);
	}

	/* always specify this */
	strbuf_printf(sb, "PRAGMA cipher_compatibility = %d;", cc.compatibility);

	af(msqlite3_exec, db, sb->buf);

	af(msqlite3_exec, db, create_table_sql);

	sqlite3_close(db);
	return 0;

init_failure:
	sqlite3_close(db);
	remove(db_path);
	remove(ky_path);
	return 2;
}