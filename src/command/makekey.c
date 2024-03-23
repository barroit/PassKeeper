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
#include "filesys.h"
#include "rawnumop.h"

static const char *output_file;
static unsigned key_size = 32;

const char *const cmd_makekey_usages[] = {
	"pk makekey [--output <file>] [--size <sz>]",
	NULL,
};

const struct option cmd_makekey_options[] = {
	OPTION_FILENAME(0, "output", &output_file, "file to be written, default is stdout"),
	OPTION_UNSIGNED('s', "size", &key_size, "specify the key length in bytes, default is 32"),
	OPTION_END(),
};

int cmd_makekey(int argc, const char **argv, const char *prefix)
{
	parse_options(argc, argv, prefix, cmd_makekey_options, cmd_makekey_usages, PARSER_ABORT_NON_OPTION);

	FILE *fs = stdout;
	if (output_file != NULL)
	{
		prepare_file_directory(output_file);

		struct stat st;
		if (stat(output_file, &st) == 0)
		{
			if (S_ISDIR(st.st_mode))
			{
				return error("'%s' is not a regular file", output_file);
			}
			if (test_file_permission(output_file, &st, R_OK) != 0)
			{
				return error("write is not allowed on file '%s'", output_file);
			}
		}

		fs = xfopen(output_file, "w");
	}

	static char *hex;

	hex = bin2hex(random_bytes(key_size), key_size);
	fprintf(fs, "0x%s", hex);
	if (fs == stdout)
	{
		putchar('\n');
	}

	return 0;
}