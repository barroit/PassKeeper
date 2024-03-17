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

static const char *db_path;
static const char *key_path;

const char *const cmd_create_usages[] = {
	"pk init [--db-path <file>] [--key-path <file>] <options>",
	NULL,
};

const struct option cmd_create_options[] = {
	// OPTION_STRING(0, "sitename", NULL, "name", "name of a web"),
	OPTION_GROUP(""),
	OPTION_PATHNAME(0, "db-path", &db_path, "credentials file to read"),
	OPTION_PATHNAME(0, "key-path", &key_path, "encryption key file to read"),
	OPTION_END(),
};

int cmd_create(int argc, const char **argv, UNUSED const char *prefix)
{
	argc = parse_options(argc, argv, prefix, cmd_create_options, cmd_create_usages, PARSER_ABORT_NON_OPTION);
	// re

	return 0;
}