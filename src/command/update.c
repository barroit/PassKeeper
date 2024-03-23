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

static struct
{
	unsigned id;
	const char *sitename;
	const char *siteurl;
	const char *username;
	const char *password;
	const char *authtext;
	const char *bakcode;
	const char *comment;
} record = {
	.id = OPTUINT_INIT,
};

const char *const cmd_update_usages[] = {
	"pk update [<fields>] [--] <id>",
	NULL,
};

const struct option cmd_update_options[] = {
	OPTION_UNSIGNED_F('i', "record", &record.id, "id", "id points to the record to be deleted", OPTION_SHOWARGH),
	OPTION_GROUP(""),
	OPTION_STRING(0, "sitename", &record.sitename, "human readable name of a website"),
	OPTION_STRING(0, "siteurl ",  &record.siteurl,  "url that used for disambiguation"),
	OPTION_STRING(0, "username", &record.username, "identification that can be used to login"),
	OPTION_STRING(0, "password", &record.password,  "secret phrase that can be used to login"),
	OPTION_STRING(0, "authtext", &record.authtext,  "predefined question selected during account creation"),
	OPTION_STRING(0, "bakcode ",  &record.bakcode, "backup code used for account recovery purposes"),
	OPTION_STRING(0, "comment ",  &record.comment, "you just write what the fuck you want to"),
	OPTION_END(),
};

int cmd_update(int argc, const char **argv, const char *prefix)
{
	argc = parse_options(argc, argv, prefix, cmd_update_options, cmd_update_usages, 0);
	return 0;
}