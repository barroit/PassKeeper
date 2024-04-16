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
	static char *buf; /* no leak ^_^ */
	struct strlist *sl = &(struct strlist)STRLIST_INIT_NODUP;

	switch (fcount)
	{
	case 3:
		strlist_push(sl, *fmap++);
		/* FALLTHRU */
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

	buf = strlist_join(sl, " ");
	strlist_destroy(sl, true);

	return buf;
}

static void recfile_field_push(struct strbuf *sb, const char *comment, const char *field)
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
	recfile_field_push(sb, "# sitename", record.sitename);
	recfile_field_push(sb, "# siteurl",  record.siteurl);
	recfile_field_push(sb, "# username", record.username);
	recfile_field_push(sb, "# password", record.password);
	recfile_field_push(sb, "# authtext", record.authtext);
	recfile_field_push(sb, "# bakcode",  record.bakcode);
	recfile_field_push(sb, "# comment",  record.comment);

	strbuf_puts(sb, COMMON_RECORD_MESSAGE);
}

static bool line_filter(struct strlist_elem *el)
{
	return *el->str != 0 && *el->str != '#';
}

int cmd_create(int argc, const char **argv, const char *prefix)
{
	parse_options(argc, argv, prefix, cmd_create_options,
			cmd_create_usages, PARSER_ABORT_NON_OPTION);

	bool setup_editor;

	if (use_editor == USE_EDITOR_DEFAULT &&
	     (!have_field(record.sitename) ||
	       !have_field(record.username) ||
	        !have_field(record.password))
	   )
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
	else if (!have_field(record.sitename) ||
		  !have_field(record.username) ||
		   !have_field(record.password))
	{
		/**
		 * ./pk create --no-nano
		 * # not use editor and missing required fields
		 */
		return error("%s missing in the required fields",
				format_missing_field_string());
	}

	if (!setup_editor)
	{
		goto arulabel;
	}

	const char *recfile;
	int recfildes;

	struct strbuf *sb = STRBUF_INIT_PTR;

	recfile = get_pk_recfile();
	prepare_file_directory(recfile);

	recfildes = xopen(recfile, O_RDWR | O_CREAT | O_TRUNC, FILCRT_BIT);

	format_recfile_content(sb);
	xwrite(recfildes, sb->buf, sb->length);

	strbuf_trunc(sb);

	// if (edit_file(recfile) != 0)
	// {
	// 	exit(EXIT_FAILURE);
	// }

	off_t recfilesz;
	char *recbuf;

	recfilesz = xlseek(recfildes, 0, SEEK_END);

	if (recfilesz == 0)
	{
		goto canceled;
	}

	xlseek(recfildes, 0, SEEK_SET); /* set starting position
					   to the beginning of the file */

	recbuf = xmalloc(recfilesz + 1);

	if (xread(recfildes, recbuf, recfilesz) == 0)
	{
		goto canceled;
	}

	recbuf[recfilesz] = 0;

	struct strlist *sl = &(struct strlist)STRLIST_INIT_DUP;

	strlist_split(sl, recbuf, '\n', -1);
	strlist_filter(sl, line_filter, false);

	free(recbuf);
	close(recfildes);
	strlist_destroy(sl, false);
	strbuf_destroy(sb);

arulabel:
	return 0;

canceled:
	puts("Aborting creation due to empty record.");
	exit(EXIT_SUCCESS);
}
