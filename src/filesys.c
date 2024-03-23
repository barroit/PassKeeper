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
#include "rawnumop.h"

static inline bool is_absolute_path(const char *path)
{
#ifdef POSIX
	return *path && *path == '/';
#else
	return *path && in_range(*path, 'A', 'Z', true) && path[1] == ':';
#endif
}

char *prefix_filename(const char *prefix, const char *filename)
{
	if (is_absolute_path(filename))
	{
		return xmemdup_str(filename, strlen(filename));
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

	strbuf_printf(sb, DIRSEP"%s", filename);

	return sb->buf;
}

#ifdef POSIX
/* check b1 has b2 if a1 has a2 */
#define check_if_has(a1, a2, b1, b2) (((a1) & (a2)) ? ((b1) & (b2)) : 1)

int test_file_permission_st(struct stat *st, int mode)
{
	if (st->st_uid == geteuid()) /* euid matches owner id? */
	{
		if (check_if_has(mode, R_OK, st->st_mode, S_IRUSR) &&
		     check_if_has(mode, W_OK, st->st_mode, S_IWUSR) &&
		      check_if_has(mode, X_OK, st->st_mode, S_IXUSR))
		{
			return 0;
		}
	}
	else if (st->st_gid == getegid()) /* egid matches group id? */
	{
		if (check_if_has(mode, R_OK, st->st_mode, S_IRGRP) &&
		     check_if_has(mode, W_OK, st->st_mode, S_IWGRP) &&
		      check_if_has(mode, X_OK, st->st_mode, S_IXGRP))
		{
			return 0;
		}
	}
	else /* check other bits */
	{
		if (check_if_has(mode, R_OK, st->st_mode, S_IROTH) &&
		     check_if_has(mode, W_OK, st->st_mode, S_IWOTH) &&
		      check_if_has(mode, X_OK, st->st_mode, S_IXOTH))
		{
			return 0;
		}
	}

	return 1;
}
#else
int test_file_permission_ch(const char *pathname, int mode)
{
	return access(pathname, mode) == -1;
}
#endif

void prepare_file_directory(const char *pathname)
{
	char *pathcopy;
	const char *pathdir;

	pathcopy = strdup(pathname);
	pathdir = dirname(pathcopy);

	struct stat st;
	if (stat(pathdir, &st) != 0)
	{
		xmkdir(pathdir);
	}
	else if (test_file_permission(pathdir, &st, W_OK) != 0)
	{
		die("access denied by '%s'", pathdir);
	}

	free(pathcopy);
}