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
#include "rawnumop.h"
#include "atexit-chain.h"

static const char *insert_common_group_sqlstr =
	"INSERT INTO account ("
		"sitename,"
		"siteurl,"
		"username,"
		"password"
	") VALUES ("
		":sitename,"
		":siteurl,"
		":username,"
		":password"
	");";

static const char *insert_security_group_sqlstr =
	"INSERT INTO account_security ("
		"account_id,"
		"guard,"
		"recovery,"
		"memo"
	") VALUES ("
		":account_id,"
		":guard,"
		":recovery,"
		":memo"
	");";

static const char *insert_misc_group_sqlstr =
	"INSERT INTO account_misc ("
		"account_id,"
		"comment"
	") VALUES ("
		":account_id,"
		":comment"
	");";

static struct record rec;

static struct
{
	int use_editor;
	const char *key;
	const char *config;
} user = {
	.use_editor = 2,
};

const char *const cmd_create_usages[] = {
	"pk create [--[no]-nano] [<field>...]",
	NULL,
};

const struct option cmd_create_options[] = {
	OPTION_BOOLEAN('e', "nano", &user.use_editor,
			"use editor to edit records"),
	OPTION_STRING('k', "key", &user.key, "db encryption key"),
	OPTION_FILENAME(0, "config", &user.config, "cipher config file"),
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
	OPTION_PATHNAME(0, "memo", &rec.recovery,
			"screenshot of the recovery code"),
	OPTION_GROUP(""),
	OPTION_STRING(0, "comment", &rec.comment,
			"you just write what the fuck you want to"),
	OPTION_END(),
};

static void rm_recfile(void)
{
	const char *pathname;

	if ((pathname = getenv(PK_RECFILE)) != NULL)
	{
		unlink(pathname);
	}
}

int cmd_create(int argc, const char **argv, const char *prefix)
{
	parse_options(argc, argv, prefix, cmd_create_options,
			cmd_create_usages, PARSER_ABORT_NON_OPTION);

	bool setup_editor;

	setup_editor = false;
	if (user.use_editor == 2 && is_incomplete_record(&rec))
	{
		/**
		 * ./pk create --sitename="xxx" --username="xxx"
		 * # no password
		 */
		setup_editor = true;
	}
	else if (user.use_editor == 1)
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

	const char *rec_path;

	atexit_chain_push(rm_recfile);

	rec_path = force_getenv(PK_RECFILE);

	populate_record_file(rec_path, &rec);

	EXIT_ON_FAILURE(edit_file(rec_path), 0);

	read_record_file(&rec, rec_path);

	atexit_chain_pop();

setup_database:;
	struct sqlite3 *db;
	const char *cfg_path, *db_path;
	bool use_cfg, use_usrkey;

	db_path  = force_getenv(PK_CRED_DB);
	cfg_path = user.config;

	resolve_cipher_config_path(&cfg_path);

	use_cfg = !!cfg_path;
	use_usrkey = !is_blank_str(user.key);

	msqlite3_pathname = db_path;
	xsqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE, NULL);

	if (!use_cfg && !use_usrkey)
	{
		goto insert_record;
	}

	struct cipher_config cc = CC_INIT;
	struct cipher_key ck = CK_INIT;

	const char *keystr;
	size_t keylen;

	keystr = NULL;
	if (use_usrkey)
	{
		keystr = user.key;
		keylen = strlen(user.key);
	}

	if (!use_cfg)
	{
		goto apply_key;
	}

	uint8_t *cc_buf;
	int cc_fd;
	off_t cc_size;

	xio_pathname = cfg_path;
	cc_fd = xopen(cfg_path, O_RDONLY);

	if ((cc_size = xlseek(cc_fd, 0, SEEK_END)) < CIPHER_DIGEST_LENGTH)
	{
		die("Cipher config file at '%s' may be corrupted because it's "
			"too small.", cfg_path);
	}

	xlseek(cc_fd, 0, SEEK_SET);

	cc_buf = xmalloc(cc_size);
	xread(cc_fd, cc_buf, cc_size);

	close(cc_fd);

	cc_size -= CIPHER_DIGEST_LENGTH;
	if (verify_digest_sha256(cc_buf, cc_size, cc_buf + cc_size) != 0)
	{
		die("File at '%s' is not a valid config file.", cfg_path);
	}

	deserialize_cipher_config(&cc, &ck, cc_buf, cc_size);
	free(cc_buf);

	if (keystr)
	{
		goto apply_key;
	}

	if (ck.buf == NULL)
	{
		warning("Config file at '%s' affects nothing without a key.",
			  cfg_path);

		free_cipher_config(&cc, &ck);
		goto insert_record;
	}
	else if (!ck.is_binary)
	{
		keystr = (char *)ck.buf;
		keylen = ck.size;
	}
	else
	{
		keystr = bin2blob(ck.buf, ck.size);
		keylen = ck.size * 2 + 3;

		ck.buf = (uint8_t *)keystr;
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

	bool need_transaction;

	if ((need_transaction = is_need_transaction(&rec)))
	{
		xsqlite3_begin_transaction(db);
	}

	struct sqlite3_stmt *stmt;
	int64_t account_id;

	xsqlite3_prepare_v2(db, insert_common_group_sqlstr, -1, &stmt, NULL);

	bind_record_basic_column(stmt, &rec);

	xsqlite3_step(stmt);

	account_id = sqlite3_last_insert_rowid(db);

	sqlite3_finalize(stmt);

	if (have_security_group(&rec))
	{
		xsqlite3_prepare_v2(db, insert_security_group_sqlstr,
					-1, &stmt, NULL);

		bind_record_security_column(stmt, account_id, &rec);

		xsqlite3_step(stmt);

		sqlite3_finalize(stmt);
	}

	if (have_misc_group(&rec))
	{
		xsqlite3_prepare_v2(db, insert_misc_group_sqlstr,
					-1, &stmt, NULL);

		bind_record_misc_column(stmt, account_id, &rec);

		xsqlite3_step(stmt);

		sqlite3_finalize(stmt);
	}

	if (need_transaction)
	{
		xsqlite3_end_transaction(db);
	}

	return 0;
}
