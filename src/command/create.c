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
#include "strlist.h"
#include "strbuf.h"
#include "pkproc.h"
#include "filesys.h"
#include "credky.h"
#include "rawnumop.h"

static struct
{
	const char *sitename;
	const char *siteurl;
	const char *username;
	const char *password;
	const char *authtext;
	const char *bakcode;
	const char *comment;
} record;

enum creterr
{
	CRETERR_CANCELED = 1,
	CRETERR_MISSING_FIELD,
};

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
	OPTION_STRING(0, "sitename", &record.sitename,
			"human readable name of a website"),
	OPTION_STRING(0, "siteurl",  &record.siteurl,
			"url that used for disambiguation"),
	OPTION_STRING(0, "username", &record.username,
			"identification that can be used to login"),
	OPTION_STRING(0, "password", &record.password,
			"secret phrase that can be used to login"),
	OPTION_GROUP(""),
	OPTION_STRING(0, "guard", &record.authtext,
			"text to help verify this account is yours"),
	OPTION_STRING(0, "recovery",  &record.bakcode,
			"code for account recovery"),
	OPTION_STRING(0, "comment",  &record.comment,
			"you just write what the fuck you want to"),
	OPTION_GROUP(""),
	OPTION_BOOLEAN('e', "nano", &user.use_editor,
			"use editor to edit records"),
	OPTION_STRING('k', "key", &user.key, "db encryption key"),
	OPTION_FILENAME(0, "config", &user.config, "cipher config file"),
	OPTION_END(),
};

#define missing_required_field				\
	(is_blank_str(record.sitename) ||		\
	  is_blank_str(record.password))

#define SITENAME_ID ":sitename:"
#define SITEURL_ID  ":siteurl:"
#define USERNAME_ID ":username:"
#define PASSWORD_ID ":password:"
#define AUTHTEXT_ID ":authtext:"
#define BAKCODE_ID  ":bakcode:"
#define COMMENT_ID  ":comment:"

static char *format_missing_field_string(void)
{
	const char *fmap0[2], **fmap;
	unsigned fcount;

	fcount = 0;
	fmap = fmap0;

	if (is_blank_str(record.sitename))
	{
		fcount++;
		*fmap++ = "sitename";
	}
	else
	{
		fcount++;
		*fmap++ = "password";
	}

	fmap = fmap0;
	static char *buf;
	struct strlist *sl = STRLIST_INIT_PTR_NODUP;

	switch (fcount)
	{
	case 2:
		strlist_push(sl, *fmap++);
		strlist_push(sl, "and");
		/* FALLTHRU */
	case 1:
		strlist_push(sl, *fmap);
		break;
	default:
		bug("format_missing_field_string() executed with an"
			"invalid field count '%d'", fcount);
	}

	buf = strlist_join(sl, " ", EXT_JOIN_TAIL);
	strlist_destroy(sl, false);

	return buf;
}

static void recfile_field_push(
	struct strbuf *sb, const char *comment, const char *field)
{
	strbuf_puts(sb, comment);

	if (!is_blank_str(field))
	{
		strbuf_puts(sb, field);
	}
	else
	{
		strbuf_putchar(sb, '\n');
	}

	strbuf_putchar(sb, '\n');
}

void format_recfile_content(struct strbuf *sb)
{
	recfile_field_push(sb, SITENAME_ID, record.sitename);
	recfile_field_push(sb, SITEURL_ID,  record.siteurl);
	recfile_field_push(sb, USERNAME_ID, record.username);
	recfile_field_push(sb, PASSWORD_ID, record.password);
	recfile_field_push(sb, AUTHTEXT_ID, record.authtext);
	recfile_field_push(sb, BAKCODE_ID,  record.bakcode);
	recfile_field_push(sb, COMMENT_ID,  record.comment);

	strbuf_puts(sb, COMMON_RECORD_MESSAGE);
}

static bool recfile_line_filter(struct strlist_elem *el)
{
	return *el->str != 0 && *el->str != '#';
}

static bool recfile_line_filter_all(struct strlist_elem *el)
{
	return recfile_line_filter(el) && *el->str != ':';
}

static void reassign_record(struct strlist *sl)
{
	memset(&record, 0, sizeof(record));

	size_t i, ii;
	struct
	{
		const char *key;
		const char **value;
	} fmap0[] = {
		{ SITENAME_ID, &record.sitename },
		{ SITEURL_ID,  &record.siteurl  },
		{ USERNAME_ID, &record.username },
		{ PASSWORD_ID, &record.password },
		{ AUTHTEXT_ID, &record.authtext },
		{ BAKCODE_ID,  &record.bakcode  },
		{ COMMENT_ID,  &record.comment  },
		{ NULL, NULL },
	}, *fmap;
	struct strbuf *sb = STRBUF_INIT_PTR;

	for (i = 0; i < sl->size; )
	{

	/**
	 * skip leading empty lines
	 */
	if (*sl->elvec[i].str != ':')
	{
		i++;
		continue;
	}

	fmap = fmap0;
	while (fmap->key != NULL)
	{
		if (!strcmp(sl->elvec[i].str, fmap->key))
		{
			break;
		}

		fmap++;
	}

	if (fmap->key == NULL)
	{
		i++;
		continue;
	}

	ii = i + 1;
	while (ii < sl->size && *sl->elvec[ii].str != ':')
	{
		if (*sl->elvec[ii].str == '|')
		{
			strbuf_puts(sb, sl->elvec[ii].str + 1);
		}
		else
		{
			strbuf_write(sb, sl->elvec[ii].str,
					strlen(sl->elvec[ii].str));
		}

		ii++;
	}

	if (ii == i + 1)
	{
		i++;
		continue;
	}

	i = ii;
	*fmap->value = strbuf_detach(sb);

	}

	strbuf_destroy(sb);
}

static int pcreterr(enum creterr errcode)
{
	switch (errcode)
	{
	case CRETERR_CANCELED:
		puts("Creation is aborted due to empty record.");
		return 0;
	case CRETERR_MISSING_FIELD:
		return error("%s missing in the required fields",
				format_missing_field_string());
	default:
		bug("Unknown error code '%d'", errcode);
	}
}

static void rm_recfile(void)
{
	const char *name;

	if ((name = getenv(PK_RECFILE)) != NULL)
	{
		unlink(name);
	}
}

static void editrec_die(enum creterr errcode)
{
	if (errcode != 0)
	{
		pcreterr(errcode);
	}

	exit(EXIT_FAILURE);
}

#define AE(...) AUTOEXIT(SQLITE_OK, __VA_ARGS__)

static const char *get_config_file(void)
{
	const char *file;
	struct stat st;

	file = user.config ? user.config : force_getenv(PK_CRED_KY);
	
	if (stat(file, &st) != 0)
	{
		file = NULL;
	}
	else if (!S_ISREG(st.st_mode))
	{
		warning("Config file at '%s' is not a regular file, "
			 "configuration disabled.", file);
		file = NULL;
	}
	else if (test_file_permission(file, &st, R_OK) != 0)
	{
		warning("Access was denied by config file '%s', "
			 "configuration disabled.", file);
		file = NULL;
	}

	return file;
}

int cmd_create(int argc, const char **argv, const char *prefix)
{
	parse_options(argc, argv, prefix, cmd_create_options,
			cmd_create_usages, PARSER_ABORT_NON_OPTION);

	bool setup_editor;

	setup_editor = false;
	atexit(rm_recfile);

	if (user.use_editor == 2 && missing_required_field)
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
	else if (missing_required_field)
	{
		/**
		 * ./pk create --no-nano
		 * # not use editor and missing required fields
		 */
		return pcreterr(CRETERR_MISSING_FIELD);
	}

	if (!setup_editor)
	{
		goto setup_database;
	}

	const char *rec_path;
	struct strbuf *rec_txt = STRBUF_INIT_PTR;

	rec_path = force_getenv(PK_RECFILE);
	xio_pathname = rec_path;

	format_recfile_content(rec_txt);

	set_file_content(rec_path, rec_txt->buf, rec_txt->length);
	strbuf_destroy(rec_txt);

	if (edit_file(rec_path) != 0)
	{
		editrec_die(0);
	}

	int rec_fd;
	char *rec_buf;
	off_t rec_bufsz;

	rec_fd = xopen(rec_path, O_RDONLY);

	if ((rec_bufsz = xlseek(rec_fd, 0, SEEK_END)) == 0)
	{
		close(rec_fd);
		editrec_die(CRETERR_CANCELED);
	}

	xlseek(rec_fd, 0, SEEK_SET);

	rec_buf = xmalloc(rec_bufsz + 1);
	rec_buf[rec_bufsz] = 0;

	if (xread(rec_fd, rec_buf, rec_bufsz) == 0)
	{
		close(rec_fd);
		editrec_die(CRETERR_CANCELED);
	}

	close(rec_fd);

	struct strlist *rec_line = STRLIST_INIT_PTR_DUPSTR;

	strlist_split(rec_line, rec_buf, '\n', -1);
	free(rec_buf);

	strlist_filter(rec_line, recfile_line_filter, false);

	if (rec_line->size == 0)
	{
		editrec_die(CRETERR_CANCELED);
	}

	reassign_record(rec_line);

	if (missing_required_field)
	{
		strlist_filter(rec_line, recfile_line_filter_all, false);

		if (rec_line->size == 0)
		{
			editrec_die(CRETERR_CANCELED);
		}
		else
		{
			editrec_die(CRETERR_MISSING_FIELD);
		}
	}

	strlist_destroy(rec_line, false);

setup_database:;
	struct sqlite3 *db;
	const char *cfg_path;
	bool use_cfg, use_usrkey;

	cfg_path = get_config_file();
	xio_pathname = cfg_path;

	use_cfg = !!cfg_path;
	use_usrkey = !is_blank_str(user.key);

	AE(msqlite3_open, force_getenv(PK_CRED_DB), &db);

	if (!use_cfg && !use_usrkey)
	{
		goto insert_record;
	}

	const char *keystr;
	size_t keylen;

	/**
	 * config file is optional
	 */
	if (!use_cfg)
	{
		goto apply_key;
	}

	uint8_t *cc_buf;
	int cc_fd;
	off_t cc_size;

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

	struct cipher_config cc;
	struct cipher_key ck;

	deserialize_cipher_config(&cc, &ck, cc_buf, cc_size);
	free(cc_buf);

	if (use_usrkey)
	{
		keystr = user.key;
		keylen = strlen(user.key);
	}
	else if (ck.buf == NULL)
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

	free_cipher_config(&cc, &ck);

apply_key:

insert_record:
	/* perferm db access check here */

	return 0;
}
