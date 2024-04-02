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

// #include "compat/poll.h"

// static int handle_nonblock(int fd, short poll_events, int err)
// {
// 	struct pollfd pfd;

// 	if (err != EAGAIN && err != EWOULDBLOCK)
// 	{
// 		return 0;
// 	}

// 	pfd.fd = fd;
// 	pfd.events = poll_events;

// 	poll(&pfd, 1, -1);
// 	return 1;
// }

ssize_t iwrite(int fd, const void *buf, size_t len)
{
	ssize_t nr;
	if (len > MAX_IO_SIZE)
	{
		len = MAX_IO_SIZE;
	}

	while (1)
	{
		if ((nr = write(fd, buf, len)) == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}

			// if (handle_nonblock(fd, POLLOUT, errno))
			// {
			// 	continue;
			// }
		}

		return nr;
	}
}