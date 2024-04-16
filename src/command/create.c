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

#define USE_EDITOR_DEFAULT 2
static int use_editor = USE_EDITOR_DEFAULT;

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
	OPTION_BOOLEAN('e', "nano", &use_editor, "use editor"),
	OPTION_END(),
};

static inline FORCEINLINE bool have_field(const char *field)
{
	return field && *field;
}

#define missing_required_field				\
	(!have_field(record.sitename) ||		\
	  !have_field(record.username) ||		\
	   !have_field(record.password))

#define SITENAME_ID ":sitename:"
#define SITEURL_ID  ":siteurl:"
#define USERNAME_ID ":username:"
#define PASSWORD_ID ":password:"
#define AUTHTEXT_ID ":authtext:"
#define BAKCODE_ID  ":bakcode:"
#define COMMENT_ID  ":comment:"

static char *format_missing_field_string(void)
{
	const char *fmap0[3], **fmap;
	unsigned fcount;

	fcount = 0;
	fmap = fmap0;

	if (!have_field(record.sitename))
	{
		fcount++;
		*fmap++ = "sitename";
	}
	if (!have_field(record.username))
	{
		fcount++;
		*fmap++ = "username";
	}
	if (!have_field(record.password))
	{
		fcount++;
		*fmap++ = "password";
	}

	fmap = fmap0;
	static char *buf;
	const char *ext;
	struct strlist *sl = &(struct strlist)STRLIST_INIT_NODUP;

	ext = NULL;
	switch (fcount)
	{
	case 3:
		strlist_push(sl, *fmap++)->ext = ",";
		ext = ",";
		/* FALLTHRU */
	case 2:
		strlist_push(sl, *fmap++)->ext = (char *)ext;
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

	if (have_field(field))
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

static void reassign_record(struct strlist *sl, struct strbuf *sb)
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

	for (i = 0; i < sl->size; )
	{

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
}

int cmd_create(int argc, const char **argv, const char *prefix)
{
	parse_options(argc, argv, prefix, cmd_create_options,
			cmd_create_usages, PARSER_ABORT_NON_OPTION);

	bool setup_editor;

	if (use_editor == USE_EDITOR_DEFAULT &&
	     missing_required_field)
	{
		/**
		 * ./pk create --sitename="xxx" --username="xxx"
		 * # no password
		 */
		setup_editor = true;
	}
	else if (use_editor && use_editor != USE_EDITOR_DEFAULT)
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
		goto missing_field;
	}

	if (!setup_editor)
	{
		goto create_record;
	}

	const char *recfile;
	int recfildes;


	static struct strbuf *sb;
	struct strbuf sb0 = STRBUF_INIT;
	sb = &sb0;

	recfile = get_pk_recfile();
	prepare_file_directory(recfile);

	recfildes = xopen(recfile, O_RDWR | O_CREAT | O_TRUNC,
				FILCRT_BIT);

	format_recfile_content(sb);
	xwrite(recfildes, sb->buf, sb->length);

	strbuf_trunc(sb);

	if (edit_file(recfile) != 0)
	{
		exit(EXIT_FAILURE);
	}

	off_t recfilesz;
	char *recbuf;

	recfilesz = xlseek(recfildes, 0, SEEK_END);

	if (recfilesz == 0)
	{
		goto creation_canceled;
	}

	xlseek(recfildes, 0, SEEK_SET);

	recbuf = xmalloc(recfilesz + 1);
	recbuf[recfilesz] = 0;

	if (xread(recfildes, recbuf, recfilesz) == 0)
	{
		goto creation_canceled;
	}
	close(recfildes);

	static struct strlist *sl;
	struct strlist sl0 = STRLIST_INIT_DUP;
	sl = &sl0;

	strlist_split(sl, recbuf, '\n', -1);
	free(recbuf);

	strlist_filter(sl, recfile_line_filter, false);

	if (sl->size == 0)
	{
		goto creation_canceled;
	}

	reassign_record(sl, sb);

	if (missing_required_field)
	{
		strlist_filter(sl, recfile_line_filter_all, false);

		if (sl->size == 0)
		{
			goto creation_canceled;
		}
		else
		{
			goto missing_field;
		}
	}

	strlist_destroy(sl, false);
	strbuf_destroy(sb);

create_record:
	return 0;

creation_canceled:
	puts("Creation is aborted due to empty record.");
	return 0;

missing_field:
	return error("%s missing in the required fields",
			format_missing_field_string());
}
