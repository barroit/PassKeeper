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

#ifndef PKPROC_H
#define PKPROC_H

#define NO_STDIN  1 << 0
#define NO_STDOUT 1 << 1
#define NO_STDERR 1 << 2

typedef int (*procfn_t)(const void *args);

#ifdef WINDOWS_NATIVE
enum process_type
{
	PROC_EXITED = -1,
	PROC_PROCESS = 1,
	PROC_THREAD,
};
#endif

struct process_info
{
#ifdef LINUX
	pid_t pid;
#else
	enum process_type type;
	HANDLE nuldev_handle;

	/* thread */
	HANDLE thread_handle;

	/* process */
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
#endif
	const char *program;
	unsigned fildes_flags;
};

int mkprocl(struct process_info *ctx, const char *arg, ...);

int mkprocf(struct process_info *ctx, procfn_t procfn, const void *args);

#ifdef LINUX
static inline int kill_process(struct process_info *ctx, int sig)
{
	return kill(ctx->pid, sig);
}
#else
int kill_process(struct process_info *ctx, int sig);
#endif

int finish_process(struct process_info *ctx, bool raised);

int edit_file(const char *tmp_file);

#define DEFAULT_SPINNER_PERIOD 1000 * 10 /* in 10 milliseconds */

int run_spinner(struct process_info *ctx, const char *style);

#endif /* PKPROC_H */