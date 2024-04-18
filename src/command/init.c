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

#define BKLEN      32
#define HKLEN      64
#define SALTLEN    32
#define HK_STRLEN  (HKLEN + 3)
#define SHK_STRLEN (HK_STRLEN + SALTLEN)
#define HK_MAXLEN  SHK_STRLEN + 1 /* add one for null-terminator */

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

#define is_binary_key (kt != KT_PASSPHRASE)

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
	if (user.key[0] == 'x' &&
	     user.key[1] == '\'' &&
	      user.key[keylen - 1] == '\'')
	{
		if (keylen == HK_STRLEN || keylen == SHK_STRLEN)
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

#define AF(...) AUTOFAIL(setup_failure, rescode, __VA_ARGS__);

int cmd_init(UNUSED int argc, const char **argv, const char *prefix)
{
	const char *db_path;
	bool encrypt_db;
	int rescode;
	int fd;

	size_t strkey_len;
	char *strkey;

	rescode = 0;
	fd = -1;

	parse_options(argc, argv, prefix, cmd_init_options,
			cmd_init_usages, PARSER_ABORT_NON_OPTION);

	db_path = force_getenv(PK_CRED_DB);
	if (access(db_path, F_OK) == 0)
	{
		return error("Database file '%s' already exists.", db_path);
	}
	else
	{
		prepare_file_directory(db_path);
	}

	encrypt_db = !!user.key;
	if (!encrypt_db)
	{
		goto setup_database;
	}

	enum key_type kt;
	bool use_cfg_file;

	kt = resolve_key_type();

	/* validate key */
	if (!is_binary_key && is_blank_str(user.key))
	{
		return error("Blank passphrase is not allowed.");
	}

	strkey_len = kt == KT_PKBIN ? HKLEN + 3 : strlen(user.key);
	if (strkey_len > 4096)
	{
		return error("Encryption key is too long.");
	}
	else if (kt == KT_USRBIN)
	{
		if (strkey_len != HK_STRLEN && strkey_len != SHK_STRLEN)
		{
			return error("Invalid blob key length '%"PRIuMAX"'.",
					strkey_len);
		}
		else if (!is_hexstr(user.key + 2, HKLEN))
		{
			return error("Blob key \"%s\" contains invalid char.",
					user.key);
		}
		else if (strkey_len == SHK_STRLEN &&
			  !is_saltstr(user.key + 2 + HKLEN, SALTLEN))
		{
			return error("Blob key \"%s\" contains invalid salt.",
					user.key);
		}
	}

	/* check if need to use config file */
	use_cfg_file = false;

	if (cc.kdf_algorithm && is_binary_key)
	{
		if (!string_in_array(cc.kdf_algorithm, kdf_algorithms))
		{
			return error("'%s' is not found in kdf algorithm "
					"list.", cc.kdf_algorithm);
		}

		use_cfg_file |= strcmp(cc.kdf_algorithm,
					CPRDEF_KDF_ALGORITHM);
	}

	if (cc.hmac_algorithm)
	{
		if (!string_in_array(cc.hmac_algorithm, hmac_algorithms))
		{
			return error("'%s' is not found in hmac algorithm "
					"list.", cc.hmac_algorithm);
		}

		use_cfg_file |= strcmp(cc.hmac_algorithm,
					CPRDEF_HMAC_ALGORITHM);
	}

	use_cfg_file |= cc.kdf_iter != CPRDEF_KDF_ITER && !is_binary_key;

	if (!in_range_u(cc.page_size, CPRMIN_PAGE_SIZE, CPRMAX_PAGE_SIZE, 1) ||
		!is_pow2(cc.page_size))
	{
		return error("Invalid page size '%u'.", cc.page_size);
	}
	else
	{
		use_cfg_file |= cc.page_size != CPRDEF_PAGE_SIZE;
	}

	if (!in_range_u(cc.cipher_compat, CPRMIN_COMPATIBILITY,
			 CPRMAX_COMPATIBILITY, 1))
	{
		return error("Unknown cipher compatibility '%u'.",
				cc.cipher_compat);
	}
	else
	{
		use_cfg_file |= cc.cipher_compat != CPRDEF_COMPATIBILITY;
	}

	use_cfg_file |= (is_binary_key && user.store_key) ||
			  user.store_key == 1;

	const char *cfg_path;

	cfg_path = force_getenv(PK_CRED_KY);
	if (use_cfg_file)
	{
		if (access(cfg_path, F_OK) == 0)
		{
			return error("Config file '%s' already exists", cfg_path);
		}
		else
		{
			prepare_file_directory(cfg_path);
		}
	}

	/* process key */
	uint8_t *cfgkey;
	size_t cfgkey_len;

	cfgkey = NULL;
	cfgkey_len = 0;

	strkey = xmalloc(strkey_len + 1);

	switch (kt)
	{
	case KT_PKBIN:
		char *hexkey;

		cfgkey = random_bytes(BKLEN);
		cfgkey_len = BKLEN;

		hexkey = bin2hex(xmemdup(cfgkey, BKLEN), BKLEN);
		snprintf(strkey, strkey_len + 1, "x'%s'", hexkey);

		free(hexkey);
		break;
	case KT_USRBIN:
		memcpy(strkey, user.key, strkey_len + 1);

		cfgkey = hex2bin(strdup(user.key + 2), strkey_len - 3);
		cfgkey_len = (strkey_len - 3) / 2;

		break;
	case KT_PASSPHRASE:
		memcpy(strkey, user.key, strkey_len + 1);

		cfgkey = xmemdup(user.key, strkey_len);
		cfgkey_len = strkey_len;

		break;
	default:
		bug("kt shall not be the value of '%d'", kt);
	}

	if (!user.store_key)
	{
		puts(strkey);

		free(cfgkey);
		cfgkey = NULL;
	}

	if (!use_cfg_file)
	{
		goto setup_database;
	}

	/* cc stands for cipher config */
	uint8_t *cc_buf, *cc_digest;
	size_t cc_size;

	cc_buf = serialize_cipher_config(&cc, cfgkey, cfgkey_len,
					   is_binary_key, &cc_size);

	cc_digest = digest_message_sha256(cc_buf, cc_size);

	memcpy(cc_buf + cc_size, cc_digest, DIGEST_LENGTH);
	clean_digest(cc_digest);

	cc_size += DIGEST_LENGTH;

	fd = xopen(cfg_path, O_WRONLY | O_CREAT, FILCRT_BIT);
	if (write(fd, cc_buf, cc_size) == -1)
	{
		rescode = error_errno("Couldn't write config to file '%s'",
					cfg_path);
		goto setup_failure;
	}

	close(fd);
	free(cc_buf);

setup_database:;
	struct sqlite3 *db;
	struct strbuf *sb = STRBUF_INIT_PTR;

	AF(msqlite3_open, db_path, &db);

	if (!encrypt_db)
	{
		goto setup_schema;
	}

	AF(msqlite3_key, db, strkey, strkey_len);
	free(strkey);

	if (cc.kdf_algorithm && is_binary_key)
	{
		strbuf_printf(sb, "PRAGMA cipher_kdf_algorithm = "
				"%s;", cc.kdf_algorithm);
	}

	if (cc.hmac_algorithm)
	{
		strbuf_printf(sb, "PRAGMA cipher_hmac_algorithm = "
				"%s;", cc.hmac_algorithm);
	}

	if (cc.kdf_iter != CPRDEF_KDF_ITER && is_binary_key)
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

	AF(msqlite3_exec, db, sb->buf);
	strbuf_destroy(sb);

	AF(msqlite3_exec, db, alter_table_sqlstr);

	sqlite3_close(db);
	return 0;

setup_failure:
	sqlite3_close(db);
	close(fd);

	unlink(db_path);
	unlink(cfg_path);

	return rescode;
}
