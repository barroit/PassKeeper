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
#include "security.h"
#include "pkerrno.h"

const char *xiopath = NULL;

void get_working_dir_routine(const char **out, bool force_get)
{
	static char *buf;
	size_t size;

	if (force_get)
	{
		free(buf);
	}
	else if (buf != NULL)
	{
		goto finish;
	}

	size = fixed_growth(64);
	while (39)
	{
		buf = xmalloc(size);

		if (getcwd(buf, size) == buf)
		{
			goto finish;
		}

		if (errno == ERANGE)
		{
			free(buf);
			size = fixed_growth(size);
			continue;
		}

		die_errno("Unable to get the current working directory");
	}

finish:
	*out = buf;
}

char *prefix_filename(const char *prefix, const char *filename)
{
	if (is_abs_path(filename))
	{
		return strdup(filename);
	}

	struct strbuf *sb = STRBUF_INIT_PTR;

	strbuf_write(sb, prefix, strlen(prefix));

	/**
	 * ./xxx
	 * .\xxx
	 */
	if (*filename && *filename == '.' &&
		(filename[1] == '/' || filename[1] == '\\'))
	{
		filename += 2;
	}

	strbuf_printf(sb, DIRSEPSTR"%s", filename);

	return sb->buf;
}

int avail_dir(const char *path)
{
	if (mkdir(path) == 0)
	{
		return 0;
	}

	if (errno != EEXIST)
	{
		/* see src/MEMO */
		return error_errno("connot create directory ‘%s’", path);
	}
	else if (access(path, W_OK | X_OK) != 0)
	{
		return error("permission denied by ‘%s’", path);
	}

	return 0;
}

int avail_file_dir(const char *path)
{
	char *tmp;
	const char *dir;
	int res;

	tmp = xstrdup(path);
	dir = dirname(tmp);

	res = avail_dir(dir);

	free(tmp);

	return res;
}

void populate_file(const char *path, const void *buf, size_t len)
{
	int fd;

	xiopath = path;

	fd = xopen(path, O_WRONLY | O_CREAT | O_TRUNC, FILCRT_BIT);
	xwrite(fd, buf, len);

	close(fd);
}

int access_regular(const char *name, int type)
{
	struct stat st;

	if (stat(name, &st) != 0)
	{
		return 1;
	}
	else if (!S_ISREG(st.st_mode))
	{
		errno = ENOTREG;
		return 1;
	}
	else if (test_file_mode(name, &st, type) != 0)
	{
		errno = EACCES;
		return 1;
	}

	return 0;
}
