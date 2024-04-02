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

#include "pkproc.h"

enum child_error_status
{
	CHILD_ERROR_DUP2,
	CHILD_ERROR_ERRNO,
};

struct child_error
{
	int err;
	int syserr;
};

static int child_notifier = -1;

static void child_die(enum child_error_status status)
{
	struct child_error field;

	field.err = status;
	field.syserr = errno;

	iwrite(child_notifier, &field, sizeof(struct child_error));
	_exit(EXIT_FAILURE);
}

static inline void xdup2(int fd1, int fd2)
{
	if (dup2(fd1, fd2))
	{
		child_die(CHILD_ERROR_DUP2);
	}
}

int start_process(struct process_ctx *ctx, procfn_t cb)
{
	// ctx->pid = fork();
	// open(DEVNULL, )

	if (ctx->pid == 0)
	{
		if (ctx->fildes_flags & NO_STDIN)
		{
			// xdup2()
		}

		if (ctx->fildes_flags & NO_STDOUT)
		{
			//
		}

		if (ctx->fildes_flags & NO_STDERR)
		{
			//
		}
		
		int rescode;

		rescode = cb();

		_exit(rescode);
	}

	return 0;
}