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

static inline bool is_absolute_path(const char *path)
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

#ifdef __linux__
static inline bool eu_match_stu(uid_t st_uid)
{
	return st_uid == geteuid();
}

static inline bool eu_can_rw_st(mode_t st_mode)
{
	return (st_mode & S_IRUSR) && (st_mode & S_IWUSR);
}

static inline bool eg_match_stg(gid_t st_gid)
{
	return st_gid == getegid();
}

static inline bool eg_can_rw_st(mode_t st_mode)
{
	return (st_mode & S_IRGRP) && (st_mode & S_IWGRP);
}

static inline bool eo_can_rw_st(mode_t st_mode)
{
	return (st_mode & S_IROTH) && (st_mode & S_IWOTH);
}

static enum file_test_result test_file_rw_permission(struct stat *st)
{
	if (eu_match_stu(st->st_uid)) /* euid matches owner id? */
	{
		if (!eu_can_rw_st(st->st_mode))
		{
			return F_NOT_ALLOW;
		}
	}
	else if (eg_match_stg(st->st_gid)) /* egid matches group id? */
	{
		if (!eg_can_rw_st(st->st_mode))
		{
			return F_NOT_ALLOW;
		}
	}
	else /* check other bits */
	{
		if (!eo_can_rw_st(st->st_mode))
		{
			return F_NOT_ALLOW;
		}
	}

	return 0;
}
#else
static inline enum file_test_result test_file_rw_permission(const char *filename)
{
	return access(filename, R_OK | W_OK) == -1 && F_NOT_ALLOW;
}
#endif

enum file_test_result test_file_avail(const char *filename)
{
	struct stat *st = &(struct stat){ 0 };
	errno = 0;

	/* file exists? */
	if (stat(filename, st) == -1)
	{
		return F_NOT_EXISTS;
	}

	if (!S_ISREG(st->st_mode))
	{
		return F_NOT_FILE;
	}

#ifdef __linux__
	return test_file_rw_permission(st);
#else
	return test_file_rw_permission(filename);
#endif
}