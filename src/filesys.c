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

#include "filesys.h"
#include "strbuf.h"

bool is_absolute_path(const char *path)
{
#ifdef __linux__
	return *path && *path == '/';
#else
	return *path && in_range(*path, 'A', 'Z', true) && path[1] == ':';
#endif
}

char *prefix_filename(const char *prefix, const char *filename)
{
	if (is_absolute_path(filename))
	{
		return xmemdup_str(filename, sizeof(filename));
	}

	struct strbuf *sb = STRBUF_INIT_P;

	strbuf_print(sb, prefix);

	/**
	 * ./xxx
	 * .\xxx
	 */
	if (*filename && *filename == '.' && (filename[1] == '/' || filename[1] == '\\'))
	{
		filename += 2;
	}

	strbuf_printf(sb, "/%s", filename);

	return sb->buf;
}