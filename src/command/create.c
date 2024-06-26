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
#include "handle-record.h"
#include "strlist.h"
#include "strbuf.h"
#include "pkproc.h"
#include "filesys.h"
#include "cipher-config.h"
#include "security.h"
#include "atexit-chain.h"

#define INSERT_COMMON_GROUP_SQLSTR		\
	"INSERT INTO account ("			\
		"sitename,"			\
		"siteurl,"			\
		"username,"			\
		"password"			\
	") VALUES ("				\
		":sitename,"			\
		":siteurl,"			\
		":username,"			\
		":password"			\
	");"

#define INSERT_SECURITY_GROUP_SQLSTR		\
	"INSERT INTO account_security ("	\
		"account_id,"			\
		"guard,"			\
		"recovery,"			\
		"memo"				\
	") VALUES ("				\
		":account_id,"			\
		":guard,"			\
		":recovery,"			\
		":memo"				\
	");"

#define INSERT_MISC_GROUP_SQLSTR		\
	"INSERT INTO account_misc ("		\
		"account_id,"			\
		"comment"			\
	") VALUES ("				\
		":account_id,"			\
		":comment"			\
	");"

static void rm_tmp_rec(void)
{
	unlink(tmp_rec_path);
}

int cmd_create(int argc, const char **argv, const char *prefix)
{
	struct record rec = INIT_RECORD;
	int use_cmdkey    = 0;
	int use_editor    = -1;

	const struct option cmd_create_options[] = {
		OPTION__NANO(&use_editor),
		OPTION__CMDKEY(&use_cmdkey),
		OPTION_GROUP(""),
		OPTION_STRING(0, "sitename", &rec.sitename,
				"human readable name of a website"),
		OPTION_STRING(0, "siteurl", &rec.siteurl,
				"url that used for disambiguation"),
		OPTION_STRING(0, "username", &rec.username,
				"identification that can be used to login"),
		OPTION_STRING(0, "password", &rec.password,
				"secret phrase that can be used to login"),
		OPTION_GROUP(""),
		OPTION_STRING(0, "guard", &rec.guard,
				"text to help verify this account is yours"),
		OPTION_STRING(0, "recovery", &rec.recovery,
				"code for account recovery"),
		OPTION_PATHNAME(0, "memo", &rec.memo,
				"screenshot of the recovery code"),
		OPTION_GROUP(""),
		OPTION_STRING(0, "comment", &rec.comment,
				"you just write what the fuck you want to"),
		OPTION_END(),
	};

	const char *const cmd_create_usages[] = {
		"pk create [--nano] [--cmdkey] [<field>...]",
		NULL,
	};

	parse_options(argc, argv, prefix, cmd_create_options,
			cmd_create_usages, PARSER_ABORT_NON_OPTION);

	bool setup_editor;

	setup_editor = false;
	if (use_editor == -1 && is_incomplete_record(&rec))
	{
		/**
		 * ./pk create --sitename="xxx" --username="xxx"
		 * # no password
		 */
		setup_editor = true;
	}
	else if (use_editor == 1)
	{
		/**
		 * ./pk create --sitename="xxx" --username="xxx"
		 * 	--password="xxx" --nano
		 * ./pk create --sitename="xxx" --username="xxx"
		 * 	--nano
		 * # even though it has required fields
		 */
		setup_editor = true;
	}
	else if (is_incomplete_record(&rec))
	{
		/**
		 * ./pk create --no-nano
		 * # not use editor and missing required fields
		 */
		return error("%s missing in the required fields",
				format_missing_field(&rec));
	}

	if (!setup_editor)
	{
		goto setup_database;
	}

	atexit_chain_push(rm_tmp_rec);

	populate_record_file(tmp_rec_path, &rec);

	EOE(edit_file(tmp_rec_path));

	EOE(read_record_file(&rec, tmp_rec_path));

setup_database:;
	struct sqlite3 *db;
	bool use_cipher_config;

	msqlite3_pathname = cred_db_path;
	xsqlite3_open_v2(cred_db_path, &db, SQLITE_OPEN_READWRITE, NULL);

	if (find_cipher_config(&cred_cc_path) != 0)
	{
		exit(error_errno("failed to find cipher config "
				  "‘%s’", cred_cc_path));
	}

	use_cipher_config = cred_cc_path != NULL;

	if (!use_cipher_config && !use_cmdkey)
	{
		goto insert_record;
	}

	struct cipher_config cc = CC_INIT;
	struct cipher_key ck = CK_INIT;

	const char *keystr;
	size_t keylen;

	keystr = NULL;
	if (use_cmdkey)
	{
		if ((ck.len = read_cmdkey((char **)&ck.buf,
				"[pk] key for decryption: ")) == 0)
		{
			return error("Empty keys are illegal.");
		}

		keystr = (char *)ck.buf;
		keylen = ck.len;
	}

	if (!use_cipher_config)
	{
		goto apply_key;
	}

	uint8_t *buf;
	off_t len;

	if (resolve_cipher_config(cred_cc_path, &buf, &len) != 0)
	{
		exit(error_errno("cannot resolve cipher config "
				  "‘%s’", cred_cc_path));
	}

	if (deserialize_cipher_config(&cc, &ck, buf, len) != 0)
	{
		exit(error_errno("cannot deserialize cipher config "
				  "‘%s’", cred_cc_path));
	}

	sfree(buf, len);

	if (keystr != NULL)
	{
		goto apply_key;
	}

	if (ck.buf == NULL)
	{
		warning("cipher config file at ‘%s’ affects nothing "
			 "without a key.", cred_cc_path);

		free_cipher_config(&cc, &ck);
		goto insert_record;
	}
	else if (!ck.is_binary)
	{
		keystr = (char *)ck.buf;
		keylen = ck.len;
	}
	else
	{
		keylen = bin2blob((char **)&ck.buf, ck.buf, ck.len);
		keystr = (char *)ck.buf;
	}

apply_key:
	xsqlite3_key(db, keystr, keylen);

	char *apply_cc_sqlstr;

	if ((apply_cc_sqlstr = format_apply_cc_sqlstr(&cc)) != NULL)
	{
		xsqlite3_exec(db, apply_cc_sqlstr, NULL, NULL, NULL);

		free(apply_cc_sqlstr);
	}

	free_cipher_config(&cc, &ck);

insert_record:
	xsqlite3_avail(db);

	bool have_transaction;

	atexit_chain_push(rm_journal_file);
	if ((have_transaction = is_need_transaction(&rec)))
	{
		xsqlite3_begin_transaction(db);
	}

	struct sqlite3_stmt *stmt;
	int64_t account_id;

	xsqlite3_prepare_v2(db, INSERT_COMMON_GROUP_SQLSTR, -1, &stmt, NULL);

	bind_record_basic_column(stmt, &rec);

	xsqlite3_step(stmt);

	account_id = sqlite3_last_insert_rowid(db);

	sqlite3_finalize(stmt);

	if (have_security_group(&rec))
	{
		xsqlite3_prepare_v2(db, INSERT_SECURITY_GROUP_SQLSTR,
					-1, &stmt, NULL);

		bind_record_security_column(stmt, account_id, &rec);

		xsqlite3_step(stmt);

		sqlite3_finalize(stmt);
	}

	if (have_misc_group(&rec))
	{
		xsqlite3_prepare_v2(db, INSERT_MISC_GROUP_SQLSTR,
					-1, &stmt, NULL);

		bind_record_misc_column(stmt, account_id, &rec);

		xsqlite3_step(stmt);

		sqlite3_finalize(stmt);
	}

	if (have_transaction)
	{
		xsqlite3_end_transaction(db);
	}

	sqlite3_close(db);

	/**
	 * at this point, cleanup journal
	 * file is unnecessary
	 */
	atexit_chain_pop();

	printf("A new record with rowid %"PRId64" was created.\n", account_id);
	return 0;
}
