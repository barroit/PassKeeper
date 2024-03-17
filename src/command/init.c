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

static bool with_cipher;
static const char *db_path;
static const char *key_path;

const char *const cmd_init_usages[] = {
	"pk init [--db-path <path>] [--key-path <path>]\n"
	"        [--encrypt | --no-encrypt]",
	NULL,
};

const struct option cmd_init_options[] = {
	OPTION_PATHNAME(0, "db-path", &db_path, "specify a file to store your credentials"),
	OPTION_PATHNAME(0, "key-path", &key_path, "specify a file to store your credential encryption key"),
	OPTION_BOOL(0, "encrypt", &with_cipher, "encrypt database with key"),
	/* consider adding some sqlcipher configure options like cipher type */
	OPTION_END(),
};

int cmd_init(int argc, const char **argv, UNUSED const char *prefix)
{
	argc = parse_options(argc, argv, prefix, cmd_init_options, cmd_init_usages, PARSER_ABORT_NON_OPTION);

	return 0;
}