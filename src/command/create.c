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
#include "pkproc.h"

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
	OPTION_STRING(0, "recovery",  &record.bakcode, "code for account recovery"),
	OPTION_STRING(0, "comment",  &record.comment,
			"you just write what the fuck you want to"),
	OPTION_GROUP(""),
	OPTION_BOOLEAN('e', "nano", &use_editor, "use editor"),
	OPTION_END(),
};

#define MISSING_FIELD(f) !(record.f && *record.f)
#define missing_sitename MISSING_FIELD(sitename)
#define missing_username MISSING_FIELD(username)
#define missing_password MISSING_FIELD(password)

static char *format_misfld_string(void)
{
	const char *fmap0[3], **fmap;
	unsigned fcount;
	char *buf;

	fcount = 0;
	fmap = fmap0;

	if (missing_sitename)
	{
		fcount++;
		*fmap++ = "sitename";
	}
	if (missing_username)
	{
		fcount++;
		*fmap++ = "username";
	}
	if (missing_password)
	{
		fcount++;
		*fmap++ = "password";
	}

	fmap = fmap0;
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
		bug("format_misfld_string() executed with an"
			"invalid field count '%d'", fcount);
	}

	buf = strlist_join(sl, " ");
	strlist_destroy(sl, true);

	return buf;
}

int cmd_create(int argc, const char **argv, const char *prefix)
{
	parse_options(argc, argv, prefix, cmd_create_options,
			cmd_create_usages, PARSER_ABORT_NON_OPTION);

	bool setup_editor;

	if (use_editor == USE_EDITOR_DEFAULT &&
	     (missing_sitename || missing_username || missing_password))
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
	else if (missing_sitename || missing_username || missing_password)
	{
		/**
		 * ./pk create --no-nano
		 * # not use editor and missing required fields
		*/
		return error("%s missing in the required fields", format_misfld_string());
	}

	printf("use editor %d\nsetup editor: %d\n", use_editor, setup_editor);
	return 0;
}