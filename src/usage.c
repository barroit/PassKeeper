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

static void vreportf(const char *prefix, const char *fmt, va_list ap)
{
	char buf_head[2048], *content_head, *content_tail; 

	content_head = buf_head + strlen(prefix);
	content_tail = content_head;

	memcpy(buf_head, prefix, content_head - buf_head);
	content_tail += vsnprintf(content_head, buf_head + sizeof(buf_head) - content_head, fmt, ap);

	if (content_tail < content_head)
	{
		*content_head = 0;
		content_tail = content_head;
	}

	*content_tail++ = '\n';
	*content_tail = 0;

	fflush(stderr);
	fputs(buf_head, stderr);
}

int warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vreportf("warning: ", fmt, ap);
	va_end(ap);

	return 0;
}

int error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vreportf("error: ", fmt, ap);
	va_end(ap);

	return -1;
}

void die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vreportf("fatal: ", fmt, ap);
	exit(EXIT_FAILURE);
}

static void bug_flvp(const char *file, int line, const char *fmt, va_list ap)
{
	char prefix[256];

	snprintf(prefix, sizeof(prefix), "BUG: %s:%d: ", file, line);
	vreportf(prefix, fmt, ap);
}

void bug_fl(const char *file, int line, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	bug_flvp(file, line, fmt, ap);

	exit(EXIT_FAILURE);
}

void report_openssl_error(void)
{
	unsigned long errcode;
	const char *reason;

	errcode = ERR_get_error();
	if (errcode == 0)
	{
		bug("calling report_openssl_error() without actual error does not make sense");
	}

	reason = ERR_reason_error_string(errcode);

	die("openssl reports '%s'", reason);
}