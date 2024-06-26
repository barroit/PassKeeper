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

#define INIT_TABLE_SQLSTR						\
	"CREATE TABLE account ("					\
		"id       INTEGER PRIMARY KEY AUTOINCREMENT,"		\
		"sitename TEXT NOT NULL,"				\
		"alias    TEXT,"					\
		"siteurl  TEXT,"					\
		"username TEXT,"					\
		"password TEXT NOT NULL,"				\
		"sqltime  DATETIME DEFAULT (datetime(‘now’, ‘utc’)),"	\
		"modtime  DATETIME"					\
	");"								\
									\
	"CREATE INDEX idx_sitename ON account(sitename);"		\
									\
	"CREATE TABLE account_security ("				\
		"account_id INTEGER PRIMARY KEY,"			\
		"guard      TEXT,"					\
		"recovery   TEXT,"					\
		"memo       BLOB,"					\
		"FOREIGN KEY (account_id) REFERENCES account(id) "	\
			"ON DELETE CASCADE"				\
	");"								\
									\
	"CREATE TABLE account_misc ("					\
		"account_id INTEGER PRIMARY KEY,"			\
		"comment    TEXT,"					\
		"FOREIGN KEY (account_id) REFERENCES account(id) "	\
			"ON DELETE CASCADE"				\
	");"

static void avail_file_path_or_die(
	const char *type, const char *path, bool force)
{
	if (access(path, F_OK) == 0)
	{
		if (!force)
		{
			exit(error("%s ‘%s’ already exists", type, path));
		}
		else if (unlink(path) != 0)
		{
			exit(error_errno("connot remove %s ‘%s’", type, path));
		}
	}

	avail_file_dir_or_die(path);
}

static size_t request_cmdkey(char **key)
{
	char  *cmdkey_buf1, *cmdkey_buf2;
	size_t cmdkey_len1,  cmdkey_len2;
	unsigned retry_count;

	retry_count = 0;
retry:
	if ((cmdkey_len1 =
		read_cmdkey(&cmdkey_buf1, "[pk] key for encryption: ")) == 0)
	{
		im_putchar('\n');
		error("No key was provided.");

		if (retry_count > 0)
		{
			note("%u key entry attempt%s made.",
				retry_count, retry_count > 1 ? "s" : "");
		}
		else
		{
			note("A key is required.");
		}

		exit(EXIT_FAILURE);
	}

	cmdkey_len2 = read_cmdkey(&cmdkey_buf2, "\n[pk] confirm key: ");
	im_putchar('\n');

	if (cmdkey_len1 != cmdkey_len2 || strcmp(cmdkey_buf1, cmdkey_buf2))
	{
		sfree(cmdkey_buf1, cmdkey_len1);
		sfree(cmdkey_buf2, cmdkey_len2);

		im_fputs("Password does not match previous, "
			  "try again.\n", stderr);

		clearerr(stdin);
		retry_count++;
		goto retry;
	}

	sfree(cmdkey_buf2, cmdkey_len2);

	*key = cmdkey_buf1;
	return cmdkey_len1;
}

static void persist_cipher_config(
	const struct cipher_config *cc, const struct cipher_key *ck)
{
	uint8_t *buf, *digest;
	size_t len;

	buf = serialize_cipher_config(cc, ck, &len);
	digest = digest_message_sha256(buf, len);

	memcpy(buf + len, digest, CIPHER_DIGEST_LENGTH);
	clean_digest(digest);

	len += CIPHER_DIGEST_LENGTH;

	populate_file(cred_cc_path, buf, len);
	sfree(buf, len);
}
 
static char **keybuf_ref;

static void destroy_key(void)
{
	if (keybuf_ref != NULL)
	{
		sfree(*keybuf_ref, strlen(*keybuf_ref));
	}
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
	int use_encryption = 0;
	int use_cmdkey     = 0;
	int remember_key   = -1;
	int force_create   = 0;

	struct cipher_config cc = CC_INIT;

	const struct option cmd_init_options[] = {
		OPTION_COUNTUP('e', "encrypt", &use_encryption,
				"encrypt database"),
		OPTION__CMDKEY(&use_cmdkey),
		OPTION_SWITCH(0, "remember", &remember_key,
				"store key"),
		OPTION_COUNTUP('f', "force", &force_create,
				"ignore existing files"),
		OPTION_GROUP(""),
		OPTION_STRING(0, "kdf-algorithm", &cc.kdf_algorithm,
				"KDF algorithm used to generate "
				 "encryption key for database"),
		OPTION_STRING(0, "hmac-algorithm", &cc.hmac_algorithm,
				"HMAC algorithm used to detect "
				 "illegal data tampering"),
		OPTION_UNSIGNED(0, "cipher-compat", &cc.compatibility,
				 "version of api to used"),
		OPTION_UNSIGNED(0, "page-size", &cc.page_size,
				"size of a page"),
		OPTION_UNSIGNED(0, "kdf-iter", &cc.kdf_iter,
				 "key derivation iteration times"),
		OPTION_END(),
	};

	const char *const cmd_init_usages[] = {
		"pk init [--encrypt] [--cmdkey] [--[no]-remember] [--force] \n"
		"        [<options>]",
		NULL,
	};

	parse_options(argc, argv, prefix, cmd_init_options,
			cmd_init_usages, PARSER_ABORT_NON_OPTION);

	avail_file_path_or_die("database", cred_db_path, force_create);

	use_encryption |= use_cmdkey;
	if (!use_encryption)
	{
		goto setup_database;
	}

	char  *keybuf;
	size_t keylen;

	if (use_cmdkey)
	{
		keylen = request_cmdkey(&keybuf);
	}
	else
	{
		uint8_t *binkey;

		EOE(random_bytes(&binkey, BINKEY_LEN));
		keylen = bin2blob(&keybuf, binkey, BINKEY_LEN);
	}

	bool use_cc, use_passphrase;

	keybuf_ref = &keybuf;
	atexit_chain_push(destroy_key);

	use_cc = false;
	use_passphrase = !is_blob_key(keybuf, keylen);

	/* kdf algorithm */
	if (cc.kdf_algorithm != NULL)
	{
/* START IF */
	if (!use_passphrase)
	{
		warning("Setting the KDF algorithm on a "
			 "non-passphrase key has no effect.");
		cc.kdf_algorithm = NULL;
	}
	else if (!is_cc_kdf_algorithm(cc.kdf_algorithm))
	{
		exit(error("invalid KDF algorithm ‘%s’", cc.kdf_algorithm));
	}
	else
	{
		use_cc |= strcmp(cc.kdf_algorithm, CPRDEF_KDF_ALGORITHM);
	}
/* END IF */
	}

	/* hmac algorithm */
	if (cc.hmac_algorithm != NULL)
	{
/* START IF */
	if (!is_cc_hmac_algorithm(cc.hmac_algorithm))
	{
		exit(error("invalid HMAC algorithm ‘%s’", cc.hmac_algorithm));
	}
	else
	{
		use_cc |= strcmp(cc.hmac_algorithm, CPRDEF_HMAC_ALGORITHM);
	}
/* END IF */
	}

	/* kdf iter */
	if (cc.kdf_iter == CPRDEF_KDF_ITER);
	else if (!use_passphrase)
	{
		warning("Setting the KDF iteration times on a "
			  "non-passphrase key has no effect.");
		cc.kdf_iter = CPRDEF_KDF_ITER;
	}
	else
	{
		use_cc |= true;
	}

	/* page size */
	if (!is_cc_page_size(cc.page_size))
	{
		exit(error("invalid page size ‘%u’", cc.page_size));
	}
	use_cc |= cc.page_size != CPRDEF_PAGE_SIZE;

	/* compatibility */
	if (!is_cc_compatibility(cc.compatibility))
	{
		exit(error("invalid cipher compatibility "
			    "‘%u’", cc.compatibility));
	}
	use_cc |= cc.compatibility != CPRDEF_COMPATIBILITY;

	/**
	 * non passphrase keys are remembered by default, passphrase keys
	 * are remembered only if the user specifies --remember
	 */
	use_cc |= (!use_passphrase && remember_key) || remember_key == 1;

	atexit_chain_pop(/* destroy_key */);

	if (!use_cc)
	{
		goto setup_database;
	}

	avail_file_path_or_die("cipher config", cred_cc_path, force_create);

	struct cipher_key ck = CK_INIT;

	if (!remember_key && !use_cmdkey)
	{
		puts(keybuf);
	}
	else if (use_passphrase)
	{
		ck.buf = xmemdup(keybuf, keylen + 1);
		ck.len = keylen;
		ck.is_binary = false;
	}
	else
	{
		ck.len = blob2bin(&ck.buf, strdup(keybuf), keylen);
		ck.is_binary = true;
	}

	atexit_chain_push(rm_cred_cc);

	persist_cipher_config(&cc, &ck);

	sfree(ck.buf, ck.len);

setup_database:;
	struct sqlite3 *db;

	atexit_chain_push(rm_cred_db);
	msqlite3_pathname = cred_db_path;

	xsqlite3_open(cred_db_path, &db);

	if (use_encryption)
	{
		char *apply_cc_sqlstr;

		xsqlite3_key(db, keybuf, keylen);

		if ((apply_cc_sqlstr = format_apply_cc_sqlstr(&cc)) != NULL)
		{
			xsqlite3_exec(db, apply_cc_sqlstr, NULL, NULL, NULL);

			free(apply_cc_sqlstr);
		}

		sfree(keybuf, keylen);
	}

	xsqlite3_exec(db, INIT_TABLE_SQLSTR, NULL, NULL, NULL);

	sqlite3_close(db);

	/**
	 * at this point, we know that this command execution succeeds,
	 * calling pop() twice because we need to remove atexit function
	 * of both cipher config file and db file
	 */
	atexit_chain_pop(/* rm_cred_db */);
	atexit_chain_pop(/* rm_cred_cc or NULL */);

	return 0;
}
