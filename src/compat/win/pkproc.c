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

int mkprocl(struct process_info *ctx, const char *arg0, ...)
{
	va_list ap;
	int rescode, errnum;
	HANDLE nuldev;

	struct strbuf *sb = STRBUF_INIT_P;
	const char *arg;
	LPSTR argv;
	
	ctx->si.cb = sizeof(STARTUPINFO);
	ctx->si.dwFlags = STARTF_USESTDHANDLES;

	redirect_stdio(ctx);

	va_start(ap, arg0);

	strbuf_printf(sb, "\"%s\" ", arg0);
	while ((arg = va_arg(ap, const char *)) != NULL)
	{
		strbuf_printf(sb, "\"%s\" ", arg);
	}

	strbuf_trim_end(sb);
	argv = strbuf_detach(sb);

	rescode = 0;
	if (!CreateProcess(NULL, argv, NULL, NULL, FALSE, 0, NULL, NULL, &ctx->si, &ctx->pi))
	{
		rescode = error_winerr("CreateProcess() failed");

		if (ctx->fildes_flags)
		{
			CloseHandle(ctx->nuldev_handle);
		}
	}

	free(argv);
	va_end(ap);

	return rescode;
}

int mkprocf(struct process_info *ctx, procfn_t procfn, const void *args)
{
	return 0;
}

int kill_process(struct process_info *ctx, int sig)
{
	return 0;
}

int finish_process(struct process_info *ctx, UNUSED bool raised)
{
	DWORD rescode, errnum;

	switch (rescode = WaitForSingleObject(ctx->pi.hProcess, INFINITE))
	{
	case WAIT_FAILED:
		errnum = GetLastError();
		rescode = error_winerr("unable to wait for %s to terminate", ctx->program);
		/* FALLTHRU */
	case WAIT_OBJECT_0:
		break;
	default:
		bug("unexpected status of %s (%ld)", ctx->program, rescode);
	}

	if (ctx->fildes_flags)
	{
		CloseHandle(ctx->nuldev_handle);
	}

	CloseHandle(ctx->pi.hProcess);
	CloseHandle(ctx->pi.hThread);

	errno = errnum;
	return rescode;
}