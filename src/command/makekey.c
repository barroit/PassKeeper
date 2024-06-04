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
#include "filesys.h"
#include "security.h"

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
	parse_options(argc, argv, prefix, cmd_makekey_options,
			cmd_makekey_usages, PARSER_ABORT_NON_OPTION);

	int fd;
	size_t hexkey_len;

	hexkey_len = key_size * 2;
	char buf0[hexkey_len + 3] /* 0x and \n */, *buf, *hexkey;

	fd = STDOUT_FILENO;
	buf = buf0;

	if (output_file != NULL)
	{
		avail_file_dir_st(output_file);

		fd = xopen(output_file, O_WRONLY | O_CREAT | O_TRUNC,
				FILCRT_BIT);
	}

	uint8_t *binkey;

	EOE(random_bytes(&binkey, BINKEY_LEN));
	bin2hex(&hexkey, binkey, key_size);

	*buf++ = '0';
	*buf++ = 'x';
	memcpy(buf, hexkey, hexkey_len);
	buf += hexkey_len;
	*buf = '\n';

	xwrite(fd, buf0, sizeof(buf0));

	exit(EXIT_SUCCESS);
}
