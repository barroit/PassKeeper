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
#include "strlist.h"

enum child_errnum
{
	ERROR_DUP2,
	ERROR_EXEC,
};

struct child_error
{
	enum child_errnum chlerr;
	int syserr;
	const void *errext;
};

static int errnot_fd = -1;

static void child_die(enum child_errnum chlerr, const void *errext)
{
	struct child_error errobj = {
		.chlerr = chlerr,
		.syserr = errno,
		.errext = errext,
	};

	iwrite(errnot_fd, &errobj, sizeof(struct child_error));
	_exit(EXIT_FAILURE);
}

static void idup2(int fd1, int fd2)
{
	if (dup2(fd1, fd2) < 0)
	{
		child_die(ERROR_DUP2, (int []){ fd1, fd2 });
	}
}

static inline void set_cloexec(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFD)) >= 0)
	{
		fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
	}
}

/**
 * this function should executed BEFORE fork()
 */
static void make_process_fildes(int *nulfd, int errnot_pipe[2])
{
	*nulfd = xopen(NULDEV, O_RDWR | O_CLOEXEC);

	if (pipe(errnot_pipe))
	{
		errnot_pipe[0] = -1;
		errnot_pipe[1] = -1;
	}
}

/**
 * this function should executed IN child process
 */
static void redirect_stdio(unsigned fildes_flags, int nulfd)
{
	if (fildes_flags & NO_STDIN)
	{
		idup2(nulfd, STDIN_FILENO);
	}

	if (fildes_flags & NO_STDOUT)
	{
		idup2(nulfd, STDOUT_FILENO);
	}

	if (fildes_flags & NO_STDERR)
	{
		idup2(nulfd, STDERR_FILENO);
	}
}

static void error_chlerr(struct child_error *errobj)
{
	errno = errobj->syserr;

	switch (errobj->chlerr)
	{
	case ERROR_DUP2:
		const int *fd;

		fd = errobj->errext;
		warning_errno("failed to duplicate 'FD %d' to 'FD %d'", fd[0], fd[1]);
		break;
	case ERROR_EXEC:
		const char *program;

		program = errobj->errext;
		warning_errno("failed to execute command '%s'", program);
		break;
	default:
		bug("child errnum should not be the value of '%d'", errobj->chlerr);
	}

	errno = errobj->syserr;
}

static int check_child_died(struct process_info *ctx, int errnot_pipe[2])
{
	struct child_error errobj;

	if (iread(errnot_pipe[0], &errobj,
		sizeof(struct child_error)) == sizeof(struct child_error))
	{
		/* failure occurred between fork() and _exit() */
		finish_process(ctx, false);
		error_chlerr(&errobj);
		ctx->pid = -1;
		return 1;
	}

	return 0;
}

/**
 * this function should executed IN parent process AFTER fork()
 */
static void after_fork(
	struct process_info *ctx, int nulfd, int errnot_pipe[2], int *errnum)
{
	if (ctx->pid < 0)
	{
		warning_errno("cannot fork() for %s", ctx->program);
	}

	close(errnot_pipe[1]);
	if (check_child_died(ctx, errnot_pipe))
	{
		*errnum = errno;
	}

	close(errnot_pipe[0]);
	close(nulfd);
}

int mkprocl(struct process_info *ctx, const char *arg0, ...)
{
	va_list ap;
	const char *arg;
	int nulfd, errnot_pipe[2];
	int errnum;

	va_start(ap, arg0);

	make_process_fildes(&nulfd, errnot_pipe);

	ctx->pid = fork();
	errnum = errno;

	if (ctx->pid == 0)
	{
		close(errnot_pipe[0]);
		set_cloexec(errnot_pipe[1]);
		errnot_fd = errnot_pipe[1];

		redirect_stdio(ctx->fildes_flags, nulfd);

		struct strlist *sl = &(struct strlist)STRLIST_INIT_NODUP;
		char **argv;

		while ((arg = va_arg(ap, const char *)) != NULL)
		{
			strlist_push(sl, arg);
		}

		argv = strlist_to_array(sl);

		strlist_clear(sl, false);
		va_end(ap);

		execvp(arg0, argv);

		child_die(ERROR_EXEC, arg0);
	}

	after_fork(ctx, nulfd, errnot_pipe, &errnum);

	va_end(ap);

	if (ctx->pid < 0)
	{
		errno = errnum;
		return -1;
	}

	return 0;
}

int mkprocf(struct process_info *ctx, procfn_t procfn, const void *args)
{
	int nulfd, errnot_pipe[2];
	int errnum;

	make_process_fildes(&nulfd, errnot_pipe);

	ctx->pid = fork();
	errnum = errno;

	if (ctx->pid == 0)
	{
		close(errnot_pipe[0]);
		errnot_fd = errnot_pipe[1];

		redirect_stdio(ctx->fildes_flags, nulfd);

		close(nulfd);
		close(errnot_pipe[1]);
		_exit(procfn(args));
	}

	after_fork(ctx, nulfd, errnot_pipe, &errnum);

	if (ctx->pid < 0)
	{
		errno = errnum;
		return -1;
	}

	return 0;
}

int finish_process(struct process_info *ctx, bool raised)
{
	int rescode, status, errnum;
	pid_t wpid;

	errnum = 0;
	rescode = -1;

	do
	{

	wpid = waitpid(ctx->pid, &status, 0);

	if (wpid < 0)
	{
		if (errno == EINTR)
		{
			continue;
		}

		errnum = errno;
		warning_errno("unable to wait for %s to terminate", ctx->program);
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
			warning("%s died (signal %d)", ctx->program, rescode);
			rescode += 128;
		}
	}
	else
	{
		bug("unexpected status of %s (0x%x)", ctx->program, status);
	}

	}
	while (!WIFEXITED(status) && !WIFSIGNALED(status));

	errno = errnum;
	return rescode;
}