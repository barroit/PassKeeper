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

#include "compat/poll.h"

#ifndef MAX_IO_SIZE
#define MAX_IO_SIZE_DEFAULT (8 * 1024 * 1024)
#if defined(SSIZE_MAX) && (SSIZE_MAX < MAX_IO_SIZE_DEFAULT)
#define MAX_IO_SIZE SSIZE_MAX
#else
#define MAX_IO_SIZE MAX_IO_SIZE_DEFAULT
#endif
#endif /* MAX_IO_SIZE */

static int handle_nonblock(int fd, short poll_events, int err)
{
	struct pollfd pfd;

	if (err != EAGAIN && err != EWOULDBLOCK)
	{
		return 0;
	}

	pfd.fd = fd;
	pfd.events = poll_events;

	poll(&pfd, 1, -1);
	return 1;
}

/**
 * read() and write() shall be the function provided by unistd
 * not the pk_read() and pk_write() wrapped by read and write macro
 */
#ifdef read
#undef read
#endif

#ifdef write
#undef write
#endif

ssize_t pk_read(int fd, void *buf, size_t nbytes)
{
	ssize_t nr;

	if (nbytes > MAX_IO_SIZE)
	{
		nbytes = MAX_IO_SIZE;
	}

	while (39)
	{
		if ((nr = read(fd, buf, nbytes)) < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}

			if (handle_nonblock(fd, POLLIN, errno))
			{
				continue;
			}
		}

		return nr;
	}
}

ssize_t pk_write(int fd, const void *buf, size_t n)
{
	ssize_t nr;

	if (n > MAX_IO_SIZE)
	{
		n = MAX_IO_SIZE;
	}

	while (39)
	{
		if ((nr = write(fd, buf, n)) < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}

			if (handle_nonblock(fd, POLLOUT, errno))
			{
				continue;
			}
		}

		return nr;
	}
}

int xopen(const char *file, int oflag, ...)
{
	mode_t mode;
	va_list ap;
	int fd;

	mode = 0;
	va_start(ap, oflag);
	if (oflag & O_CREAT)
	{
		mode = (mode_t)va_arg(ap, int);
	}
	va_end(ap);

	if ((fd = open(file, oflag, mode)) == -1)
	{
		die_errno("Failed to open file at ‘%s’", file);
	}

	return fd;
}

int msqlite3_exec(
	struct sqlite3 *db, const char *sql,
	int (*callback)(void *, int, char **, char **),
	void *cbargv, char **errmsg0)
{
	char *errmsg;
	int rescode;

	rescode = SQLITE_OK;
	if (sqlite3_exec(db, sql, callback, cbargv, &errmsg) != SQLITE_OK)
	{
		rescode = report_sqlite_error(sqlite3_exec, db, sql, errmsg);
	}

	if (errmsg0 == NULL)
	{
		sqlite3_free(errmsg);
	}
	else
	{
		*errmsg0 = errmsg;
	}

	return rescode;
}

int sqlite3_avail(struct sqlite3 *db)
{
	return sqlite3_exec(db, "SELECT count(*) FROM sqlite_master;",
				NULL, NULL, NULL);
}
