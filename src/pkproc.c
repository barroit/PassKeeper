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
	ERROR_DUP2,
	ERROR_ERRNO,
};

static int errnot_fd = -1;

static void child_die(enum child_error_status status)
{
	iwrite(errnot_fd, (int []){ status, errno }, sizeof(int [2]));
	_exit(EXIT_FAILURE);
}

static void idup2(int fd1, int fd2)
{
	if (dup2(fd1, fd2) < 0)
	{
		child_die(ERROR_DUP2);
	}
}

int finish_process(struct process_info *ctx, bool raised)
{
	pid_t wpid;
	int status, rescode;
	int tmp_errno;

	rescode = -1;
	tmp_errno = 0;

	do
	{
		wpid = waitpid(ctx->pid, &status, 0);

		if (wpid < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}

			tmp_errno = errno;
			error_errno("waitpid() for %s failed", ctx->program);
		}
		else if (WIFEXITED(status))
		{
			rescode = WEXITSTATUS(status);
		}
		else if (WIFSIGNALED(status))
		{
			rescode = WTERMSIG(status);

			if (!raised && rescode != SIGINT &&
				rescode != SIGQUIT && rescode != SIGPIPE)
			{
				error("%s died (signal %d)", ctx->program, rescode);
				rescode += 128;
			}
		}
		else
		{
			bug("Unexpected status (0x%x)", status);
		}
	}
	while (!WIFEXITED(status) && !WIFSIGNALED(status));

	errno = tmp_errno;
	return rescode;
}

int start_process(struct process_info *ctx, procfn_t procfn, const void *args)
{
	int buf[2];
	int nulfd, errnot_pipe[2];
	int errnum;

	nulfd = xopen(DEVNULL, O_WRONLY | O_CLOEXEC);

	if (pipe(errnot_pipe))
	{
		errnot_pipe[0] = -1;
		errnot_pipe[1] = -1;
	}

	// ctx->pid = fork();
	errnum = errno;

	if (ctx->pid == 0)
	{
		close(errnot_pipe[0]);

		errnot_fd = errnot_pipe[1];

		if (ctx->fildes_flags & NO_STDIN)
		{
			idup2(nulfd, STDIN_FILENO);
		}

		if (ctx->fildes_flags & NO_STDOUT)
		{
			idup2(nulfd, STDOUT_FILENO);
		}

		if (ctx->fildes_flags & NO_STDERR)
		{
			idup2(nulfd, STDERR_FILENO);
		}

		close(errnot_pipe[1]);
		_exit(procfn(args));
	}

	if (ctx->pid < 0)
	{
		error_errno("cannot fork() for %s", ctx->program);
	}

	close(errnot_pipe[1]);
	if (iread(errnot_pipe[0], buf, sizeof(buf)) == sizeof(buf))
	{
		/* failure occurred between fork() and _exit() */
		finish_process(ctx, false);
		errnum = errno;
		ctx->pid = -1;
	}
	close(errnot_pipe[0]);

	close(nulfd);

	if (ctx->pid < 0)
	{
		errno = errnum;
		return -1;
	}

	return 0;
}