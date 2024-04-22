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

#include "parseopt.h"
#include "filesys.h"
#include "rawnumop.h"
#include "credky.h"
#include "strlist.h"
#include "strbuf.h"

static const char *init_table_sqlstr = 
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
		"FOREIGN KEY (account_id) REFERENCES account(id) "
			"ON DELETE CASCADE"
	");"

	"CREATE TABLE account_misc ("
		"account_id INTEGER PRIMARY KEY,"
		"comment    TEXT,"
		"FOREIGN KEY (account_id) REFERENCES account(id) "
			"ON DELETE CASCADE"
	");";

static struct
{
	int store_key;
	const char *key;
} user = {
	.store_key = 2,
};

static struct cipher_config cc = {
	.kdf_iter = CPRDEF_KDF_ITER,
	.page_size = CPRDEF_PAGE_SIZE,
	.cipher_compat = CPRDEF_COMPATIBILITY,
};

const char *const cmd_init_usages[] = {
	"pk init [--encrypt[=<key>]] [<options>]",
	NULL,
};

const struct option cmd_init_options[] = {
	OPTION_OPTARG(0, "encrypt", &user.key, 1, "key",
			"password used to encryption"),
	OPTION_BOOLEAN(0, "remember", &user.store_key, "store password"),
	OPTION_GROUP(""),
	OPTION_STRING(0, "kdf-algorithm", &cc.kdf_algorithm,
			"KDF algorithm used to generate "
			 "encryption key for database"),
	OPTION_STRING(0, "hmac-algorithm", &cc.hmac_algorithm,
			"HMAC algorithm used to detect "
			 "illegal data tampering"),
	OPTION_UNSIGNED(0, "cipher-compat", &cc.cipher_compat,
			 "version of api to used"),
	OPTION_UNSIGNED(0, "page-size", &cc.page_size, "size of a page"),
	OPTION_UNSIGNED(0, "kdf-iter", &cc.kdf_iter,
			 "key derivation iteration times"),
	OPTION_END(),
};

static const char *kdf_algorithms[] = {
	CPRDEF_KDF_ALGORITHM,
	"PBKDF2HMAC_SHA256",
	"PBKDF2_HMAC_SHA1",
	NULL,
};

static const char *hmac_algorithms[] = {
	CPRDEF_HMAC_ALGORITHM,
	"HMAC_SHA256",
	"HMAC_SHA1",
	NULL,
};

enum key_type
{
	KT_UNKNOWN,
	KT_PASSPHRASE,
	KT_PKBIN,
	KT_USRBIN,
};

static enum key_type resolve_key_type(void)
{
	size_t keylen;
	if (user.key == NULL)
	{
		bug("user.key shall not be null.");
	}

	if ((intptr_t)user.key == 1)
	{
		return KT_PKBIN;
	}

	keylen = strlen(user.key);
	if (is_binkey_wrp(user.key, keylen))
	{
		if (is_binkey_len(keylen))
		{
			return KT_USRBIN;
		}
		else
		{
			warning("Encryption key is wrapped by \"x''\" but "
				 "does not have a valid raw key data length.");
			puts("note: Using passphrase.");
			return KT_PASSPHRASE;
		}
	}
	else
	{
		return KT_PASSPHRASE;
	}
}

static int precheck_file(const char *name, const char *path)
{
	if (access(path, F_OK) == 0)
	{
		return error("%s file '%s' already exists", name, path);
	}

	prepare_file_directory(path);

	return 0;
}

int validate_key(enum key_type keytype, size_t keylen)
{
	if (keytype == KT_PASSPHRASE && is_blank_str(user.key))
	{
		return error("Blank passphrase is not allowed.");
	}

	if (keytype == KT_USRBIN)
	{
		if (keylen != HK_STRLEN && keylen != SHK_STRLEN)
		{
			return error("Invalid blob key length '%"PRIuMAX"'.",
					keylen);
		}
		else if (!is_hexstr(user.key + 2, HEXKEY_LEN))
		{
			return error("Blob key \"%s\" contains invalid char.",
					user.key);
		}
		else if (keylen == SHK_STRLEN &&
			  !is_saltstr(user.key + 2 + HEXKEY_LEN, KEYSALT_LEN))
		{
			return error("Blob key \"%s\" contains invalid salt.",
					user.key);
		}
	}

	return 0;
}

int check_if_cc_needed(enum key_type keytype, bool *out)
{
	bool use_cc;

	use_cc = false;
	if (cc.kdf_algorithm && keytype != KT_PASSPHRASE)
	{
		if (!string_in_array(cc.kdf_algorithm, kdf_algorithms))
		{
			return error("'%s' is not found in kdf algorithm "
					"list.", cc.kdf_algorithm);
		}

		use_cc |= strcmp(cc.kdf_algorithm, CPRDEF_KDF_ALGORITHM);
	}

	if (cc.hmac_algorithm)
	{
		if (!string_in_array(cc.hmac_algorithm, hmac_algorithms))
		{
			return error("'%s' is not found in hmac algorithm "
					"list.", cc.hmac_algorithm);
		}

		use_cc |= strcmp(cc.hmac_algorithm, CPRDEF_HMAC_ALGORITHM);
	}

	use_cc |= cc.kdf_iter != CPRDEF_KDF_ITER && keytype == KT_PASSPHRASE;

	if (!in_range_u(cc.page_size, CPRMIN_PAGE_SIZE, CPRMAX_PAGE_SIZE, 1) ||
		!is_pow2(cc.page_size))
	{
		return error("Invalid page size '%u'.", cc.page_size);
	}
	else
	{
		use_cc |= cc.page_size != CPRDEF_PAGE_SIZE;
	}

	if (!in_range_u(cc.cipher_compat, CPRMIN_COMPATIBILITY,
			 CPRMAX_COMPATIBILITY, 1))
	{
		return error("Unknown cipher compatibility '%u'.",
				cc.cipher_compat);
	}
	else
	{
		use_cc |= cc.cipher_compat != CPRDEF_COMPATIBILITY;
	}

	use_cc |= (keytype != KT_PASSPHRASE && user.store_key) ||
			  user.store_key == 1;

	*out = use_cc;
	return 0;
}

enum cleanup_flag
{
	RM_DB_FILE = 1 << 0,
	RM_CC_FILE = 1 << 1,
};

static enum cleanup_flag cleanup_flags;

static void cleanup_files(void)
{
	if (cleanup_flags & RM_DB_FILE)
	{
		unlink(force_getenv(PK_CRED_DB));
	}

	if (cleanup_flags & RM_CC_FILE)
	{
		unlink(force_getenv(PK_CRED_KY));
	}
}

int cmd_init(UNUSED int argc, const char **argv, const char *prefix)
{
	const char *db_path;
	bool encrypt_db;

	parse_options(argc, argv, prefix, cmd_init_options,
			cmd_init_usages, PARSER_ABORT_NON_OPTION);

	encrypt_db = !!user.key;

	db_path = force_getenv(PK_CRED_DB);
	precheck_file("Database", db_path);

	atexit(cleanup_files);

	if (!encrypt_db)
	{
		goto setup_database;
	}

	const char *keystr;
	size_t keylen;
	enum key_type keytype;

	keytype = resolve_key_type();

	if (keytype == KT_PKBIN)
	{
		user.key = bin2blob(random_bytes(BINKEY_LEN), BINKEY_LEN);
	}

	keylen = strlen(user.key);
	keystr = user.key;

	EXIT_ON_FAILURE(validate_key(keytype, keylen), 0);

	bool use_cc;
	const char *cc_path;

	EXIT_ON_FAILURE(check_if_cc_needed(keytype, &use_cc), 0);
	cc_path = force_getenv(PK_CRED_KY);

	if (use_cc)
	{
		precheck_file("Config", cc_path);
	}
	else
	{
		goto setup_database;
	}

	struct cipher_key ck = { 0 };

	if (!user.store_key)
	{
		puts(keystr);
		goto setup_cc;
	}

	switch (keytype)
	{
	case KT_PKBIN:
	case KT_USRBIN:
		ck.buf = hex2bin(strdup(keystr + 2), keylen - 3);
		ck.size = (keylen - 3) / 2;
		ck.is_binary = true;

		break;
	case KT_PASSPHRASE:
		ck.buf = xmemdup(keystr, keylen + 1);
		ck.size = keylen;
		ck.is_binary = false;

		break;
	default:
		bug("keytype shall not be the value of '%d'", keytype);
	}

setup_cc:
	/* cc stands for cipher config */
	uint8_t *cc_buf, *cc_digest;
	size_t cc_size;
	int cc_fd;

	cc_buf = serialize_cipher_config(&cc, &ck, &cc_size);
	cc_digest = digest_message_sha256(cc_buf, cc_size);

	memcpy(cc_buf + cc_size, cc_digest, CIPHER_DIGEST_LENGTH);
	clean_digest(cc_digest);

	cc_size += CIPHER_DIGEST_LENGTH;

	xio_pathname = cc_path;
	cleanup_flags |= RM_CC_FILE;

	cc_fd = xopen(cc_path, O_WRONLY | O_CREAT, FILCRT_BIT);
	xwrite(cc_fd, cc_buf, cc_size);

	close(cc_fd);
	free(cc_buf);
	free_cipher_config(&cc, &ck);

setup_database:;
	struct sqlite3 *db;
	struct strbuf *sb = STRBUF_INIT_PTR;

	cleanup_flags |= RM_DB_FILE;
	msqlite3_pathname = db_path;
	EXIT_ON_FAILURE(msqlite3_open(db_path, &db), SQLITE_OK);

	if (!encrypt_db)
	{
		goto setup_schema;
	}

	EXIT_ON_FAILURE(msqlite3_key(db, keystr, keylen), SQLITE_OK);

	if (cc.kdf_algorithm && keytype != KT_PASSPHRASE)
	{
		strbuf_printf(sb, "PRAGMA cipher_kdf_algorithm = "
				"%s;", cc.kdf_algorithm);
	}

	if (cc.hmac_algorithm)
	{
		strbuf_printf(sb, "PRAGMA cipher_hmac_algorithm = "
				"%s;", cc.hmac_algorithm);
	}

	if (cc.kdf_iter != CPRDEF_KDF_ITER && keytype != KT_PASSPHRASE)
	{
		strbuf_printf(sb, "PRAGMA kdf_iter = %d;", cc.kdf_iter);
	}

	if (cc.page_size != CPRDEF_PAGE_SIZE)
	{
		strbuf_printf(sb, "PRAGMA cipher_page_size = "
				"%d;", cc.page_size);
	}

setup_schema:
	/* always specify this */
	strbuf_printf(sb, "PRAGMA cipher_compatibility = "
			"%d;", cc.cipher_compat);

	EXIT_ON_FAILURE(msqlite3_exec(db, sb->buf, NULL, NULL), SQLITE_OK);
	strbuf_destroy(sb);

	EXIT_ON_FAILURE(msqlite3_exec(db, init_table_sqlstr, NULL, NULL),
			 SQLITE_OK);

	sqlite3_close(db);
	cleanup_flags = 0;

	return 0;
}
