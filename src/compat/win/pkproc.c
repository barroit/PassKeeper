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
#include "strbuf.h"

static void redirect_stdio(struct process_info *ctx)
{
	HANDLE nuldev;

	if (ctx->fildes_flags)
	{
		nuldev = xCreateFile(NULDEV, GENERIC_READ | GENERIC_WRITE, 0,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		ctx->nuldev_handle = nuldev;
	}

	if (ctx->fildes_flags & NO_STDIN)
	{
		ctx->si.hStdInput = nuldev;
	}

	if (ctx->fildes_flags & NO_STDOUT)
	{
		ctx->si.hStdOutput = nuldev;
	}

	if (ctx->fildes_flags & NO_STDERR)
	{
		ctx->si.hStdError = nuldev;
	}
}

static inline void close_nuldev(struct process_info *ctx)
{
	if (ctx->fildes_flags)
	{
		CloseHandle(ctx->nuldev_handle);
	}
}

int mkprocl(struct process_info *ctx, const char *arg0, ...)
{
	ctx->type = PROC_PROCESS;

	va_list ap;
	int rescode;

	struct strbuf *sb = STRBUF_INIT_PTR;
	const char *arg;

	ctx->si.cb = sizeof(STARTUPINFO);
	ctx->si.dwFlags = STARTF_USESTDHANDLES;

	redirect_stdio(ctx);

	va_start(ap, arg0);

	strbuf_printf(sb, "\"%s\" ", arg0);

	/**
	 * no need to pass the program name twice
	 * so we skip the first va argument
	 */
	arg = va_arg(ap, const char *);

	while ((arg = va_arg(ap, const char *)) != NULL)
	{
		strbuf_printf(sb, "%s ", arg);
	}

	strbuf_trim_end(sb);

	rescode = 0;
	if (!CreateProcess(NULL, sb->buf, NULL, NULL, FALSE, 0,
				NULL, NULL, &ctx->si, &ctx->pi))
	{
		warning_winerr("failed to start the program '%s'",
				ctx->program); /* errno set here */
		rescode = -1;
		close_nuldev(ctx);
	}

	va_end(ap);
	strbuf_destroy(sb);

	return rescode;
}

int mkprocf(struct process_info *ctx, procfn_t procfn, const void *args)
{
	int rescode;
	ctx->type = PROC_THREAD;

	redirect_stdio(ctx);

	rescode = 0;
	if ((ctx->thread_handle =
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)procfn,
			      (LPVOID)args, 0, NULL)) == NULL)
	{
		warning_winerr("failed to start a new thread for program '%s'",
				ctx->program);
		rescode = -1;
		close_nuldev(ctx);
	}

	return rescode;
}

int kill_process(struct process_info *ctx, int sig)
{
	DWORD rescode;

	close_nuldev(ctx);

	rescode = 0;
	switch (ctx->type)
	{
	case PROC_PROCESS:
		if (!TerminateProcess(ctx->pi.hProcess, sig + 128))
		{
			warning_winerr("failed to terminate the program '%s'",
					ctx->program);
			rescode = -1;
		}
		break;
	case PROC_THREAD:
		if (!TerminateThread(ctx->thread_handle, sig + 128))
		{
			warning_winerr("failed to terminate the thread running '%s'",
					ctx->program);
			rescode = -1;
		}
		break;
	case PROC_EXITED:
		bug("ctx should not be the type of 'exited'");
	default:
		bug("unknown proctype: %d", ctx->type);
	}

	ctx->type = PROC_EXITED;
	/* no need to update errno */
	return rescode;
}

int finish_process(struct process_info *ctx, UNUSED bool raised)
{
	if (ctx->type == PROC_EXITED)
	{
		return 0;
	}

	DWORD rescode, errnum;

	errnum = 0;
	switch (rescode = WaitForSingleObject(ctx->pi.hProcess, INFINITE))
	{
	case WAIT_FAILED:
		warning_winerr("failed to wait for %s to terminate", ctx->program);
		rescode = -1;
		errnum = errno;
		/* FALLTHRU */
	case WAIT_OBJECT_0:
		close_nuldev(ctx);
		CloseHandle(ctx->pi.hProcess);
		CloseHandle(ctx->pi.hThread);
		break;
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
		bug("unexpected status of %s (%ld)", ctx->program, rescode);
	}

	errno = errnum;
	return rescode;
}
