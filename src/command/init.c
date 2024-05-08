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

#include "parse-option.h"
#include "filesys.h"
#include "security.h"
#include "cipher-config.h"
#include "strlist.h"
#include "strbuf.h"
#include "atexit-chain.h"

static const char *init_table_sqlstr = 
	"CREATE TABLE account ("
		"id       INTEGER PRIMARY KEY AUTOINCREMENT,"
		"sitename TEXT NOT NULL,"
		"siteurl  TEXT,"
		"username TEXT,"
		"password TEXT,"
		"sqltime  DATETIME DEFAULT (datetime('now', 'utc')),"
		"modtime  DATETIME"
	");"

	"CREATE INDEX idx_sitename ON account(sitename);"

	"CREATE TABLE account_security ("
		"account_id INTEGER PRIMARY KEY,"
		"guard      TEXT,"
		"recovery   TEXT,"
		"memo       BLOB,"
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
	.compatibility = CPRDEF_COMPATIBILITY,
};

const char *const cmd_init_usages[] = {
	"pk init [--encrypt[=<key>]] [<options>]",
	NULL,
};

const struct option cmd_init_options[] = {
	OPTION_OPTARG(0, "encrypt", &user.key, 1, "key",
			"password used to encryption"),
	OPTION_SWITCH(0, "remember", &user.store_key, "store password"),
	OPTION_GROUP(""),
	OPTION_STRING(0, "kdf-algorithm", &cc.kdf_algorithm,
			"KDF algorithm used to generate "
			 "encryption key for database"),
	OPTION_STRING(0, "hmac-algorithm", &cc.hmac_algorithm,
			"HMAC algorithm used to detect "
			 "illegal data tampering"),
	OPTION_UNSIGNED(0, "cipher-compat", &cc.compatibility,
			 "version of api to used"),
	OPTION_UNSIGNED(0, "page-size", &cc.page_size, "size of a page"),
	OPTION_UNSIGNED(0, "kdf-iter", &cc.kdf_iter,
			 "key derivation iteration times"),
	OPTION_END(),
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
			note("Using passphrase.");
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

static int validate_key(enum key_type keytype, size_t keylen)
{
	if (keytype == KT_PASSPHRASE && is_blank_str(user.key))
	{
		return error("Blank passphrase is not allowed.");
	}

	if (keytype == KT_USRBIN)
	{
		if (keylen != HK_STRLEN && keylen != SHK_STRLEN)
		{
			/**
			 * 7.8.1 Macros for format specifiers
			 * 
			 * MS runtime does not yet understand C9x standard "ll"
			 * length specifier. It appears to treat "ll" as "l".
			 * The non-standard I64 length specifier causes warning
			 * in GCC, but understood by MS runtime functions.
			 */
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

static void process_cipher_config(enum key_type keytype, bool *out)
{
	bool use_cc;

	use_cc = false;
	if (cc.kdf_algorithm == NULL);
	else if (keytype != KT_PASSPHRASE)
	{
		warning("Setting the KDF algorithm on a "
			  "non-passphrase key has no effect.");

		cc.kdf_algorithm = NULL;
	}
	else
	{
		EXIT_ON_FAILURE(check_kdf_algorithm(cc.kdf_algorithm), 0);

		/* kdf algorithm */
		use_cc |= strcmp(cc.kdf_algorithm, CPRDEF_KDF_ALGORITHM);
	}

	if (cc.hmac_algorithm)
	{
		EXIT_ON_FAILURE(check_hmac_algorithm(cc.hmac_algorithm), 0);

		/* hmac algorithm */
		use_cc |= strcmp(cc.hmac_algorithm, CPRDEF_HMAC_ALGORITHM);
	}

	/* kdf iter */
	if (cc.kdf_iter == CPRDEF_KDF_ITER);
	else if (keytype != KT_PASSPHRASE)
	{
		warning("Setting the KDF iteration times on a "
			  "non-passphrase key has no effect.");

		cc.kdf_iter = CPRDEF_KDF_ITER;
	}
	else
	{
		use_cc |= true;
	}

	EXIT_ON_FAILURE(check_page_size(cc.page_size), 0);

	/* page size */
	use_cc |= cc.page_size != CPRDEF_PAGE_SIZE;

	EXIT_ON_FAILURE(check_compatibility(cc.compatibility), 0);

	/* compatibility */
	use_cc |= cc.compatibility != CPRDEF_COMPATIBILITY;

	use_cc |= (keytype != KT_PASSPHRASE && user.store_key) ||
			  user.store_key == 1;

	*out = use_cc;
}

static void rm_cred_db(void)
{
	unlink(cred_db_path);
}

static void rm_cred_cc(void)
{
	unlink(cred_cc_path);
}

int cmd_init(UNUSED int argc, const char **argv, const char *prefix)
{
	bool encrypt_db;

	parse_options(argc, argv, prefix, cmd_init_options,
			cmd_init_usages, PARSER_ABORT_NON_OPTION);

	encrypt_db = !!user.key;

	precheck_file("Database", cred_db_path);

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
		uint8_t *binkey;

		EXIT_ON_FAILURE(random_bytes_alloc(&binkey, BINKEY_LEN), 0);
		user.key = bin2blob(binkey, BINKEY_LEN);
	}

	keylen = strlen(user.key);
	keystr = user.key;

	EXIT_ON_FAILURE(validate_key(keytype, keylen), 0);

	bool use_cc;

	process_cipher_config(keytype, &use_cc);

	if (use_cc)
	{
		precheck_file("Config", cred_cc_path);
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
		ck.len = (keylen - 3) / 2;
		ck.is_binary = true;

		break;
	case KT_PASSPHRASE:
		ck.buf = xmemdup(keystr, keylen + 1);
		ck.len = keylen;
		ck.is_binary = false;

		break;
	default:
		bug("keytype shall not be the value of '%d'", keytype);
	}

setup_cc:;
	/* cc stands for cipher config */
	uint8_t *cc_buf, *cc_digest;
	size_t cc_size;
	int cc_fd;

	cc_buf = serialize_cipher_config(&cc, &ck, &cc_size);
	cc_digest = digest_message_sha256(cc_buf, cc_size);

	memcpy(cc_buf + cc_size, cc_digest, CIPHER_DIGEST_LENGTH);
	clean_digest(cc_digest);

	cc_size += CIPHER_DIGEST_LENGTH;

	atexit_chain_push(rm_cred_cc);
	xio_pathname = cred_cc_path;

	cc_fd = xopen(cred_cc_path, O_WRONLY | O_CREAT, FILCRT_BIT);

	xwrite(cc_fd, cc_buf, cc_size);

	close(cc_fd);
	free(cc_buf);
	free(ck.buf);

setup_database:;
	struct sqlite3 *db;

	atexit_chain_push(rm_cred_db);
	msqlite3_pathname = cred_db_path;

	xsqlite3_open(cred_db_path, &db);

	if (encrypt_db)
	{
		char *apply_cc_sqlstr;

		xsqlite3_key(db, keystr, keylen);

		if ((apply_cc_sqlstr = format_apply_cc_sqlstr(&cc)) != NULL)
		{
			xsqlite3_exec(db, apply_cc_sqlstr, NULL, NULL, NULL);

			free(apply_cc_sqlstr);
		}
	}

	xsqlite3_exec(db, init_table_sqlstr, NULL, NULL, NULL);

	sqlite3_close(db);

	/**
	 * at this point, we know that this command execution succeeds,
	 * calling pop() twice because we need to remove atexit function
	 * of both cipher config file and db file
	 */
	atexit_chain_pop();
	atexit_chain_pop();

	return 0;
}
