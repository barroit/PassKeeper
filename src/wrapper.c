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

ssize_t iread(int fd, void *buf, size_t nbytes)
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

ssize_t iwrite(int fd, const void *buf, size_t n)
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
		die_errno("Failed to open file at '%s'", file);
	}

	return fd;
}