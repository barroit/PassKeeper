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

#include "parse-options.h"
#include "strbuf.h"
#include "exteditor.h"

static struct
{
	const char *sitename;
	const char *siteurl;
	const char *username;
	const char *password;
	const char *authtext;
	const char *bakcode;
	const char *comment;
	// const char *sqltime
	// const char *modtime,
} record;

static bool use_editor;

const char *const cmd_create_usages[] = {
	"pk create <fields>",
	NULL,
};

const struct option cmd_create_options[] = {
	OPTION_STRING(0, "sitename", &record.sitename, "human readable name of a website"),
	OPTION_STRING(0, "siteurl",  &record.siteurl,  "url that used for disambiguation"),
	OPTION_STRING(0, "username", &record.username, "identification that can be used to login"),
	OPTION_STRING(0, "password", &record.password,  "secret phrase that can be used to login"),
	OPTION_GROUP(""),
	OPTION_STRING(0, "guard", &record.authtext,  "text to help verify this account is yours"),
	OPTION_STRING(0, "recovery",  &record.bakcode, "code for account recovery"),
	OPTION_STRING(0, "comment",  &record.comment, "you just write what the fuck you want to"),
	OPTION_GROUP(""),
	OPTION_BOOLEAN('e', "nano", &use_editor, "use editor"),
	OPTION_END(),
};

int cmd_create(int argc, const char **argv, const char *prefix)
{
	argc = parse_options(argc, argv, prefix, cmd_create_options, cmd_create_usages, PARSER_ABORT_NON_OPTION);
	// edit_file("file");

	fputs("test", stdout);
	fflush(stdout);
	sleep(1);
	fputs("\r    \r", stdout);

	return 0;
}