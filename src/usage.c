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

#include "security.h"

void vreportf(
	const char *prefix,
	const char *format, va_list ap,
	const char *detail)
{
	char buf0[4096], *buf1, *buf;
	size_t bufaval;

	buf1 = buf0 + strlen(prefix);
	buf = buf1;

	memcpy(buf0, prefix, buf1 - buf0);
	bufaval = (buf0 + sizeof(buf0)) - buf1;

	buf += vsnprintf(buf, bufaval, format, ap);

	/* vsnprintf() failure */
	if (buf < buf1)
	{
		/**
		 * reset buf to buf1, skip the ap
		 * and prepare to write errmsg
		 */
		buf = buf1;
	}
	else
	{
		bufaval -= buf - buf1;
	}

	if (detail && bufaval > 0)
	{
		int nr;
		if ((nr = snprintf(buf, bufaval, "; %s", detail)) < 0)
		{
			nr = 0;
		}

		buf += nr;
	}

	*buf++ = '\n';

	fflush(stderr);
	write(STDERR_FILENO, buf0, buf - buf0);
}

void note_routine(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vreportf("note: ", format, ap, NULL);
	va_end(ap);
}

void warning_routine(const char *detail, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vreportf("warning: ", format, ap, detail);
	va_end(ap);
}

int error_routine(const char *detail, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vreportf("error: ", format, ap, detail);
	va_end(ap);

	return -1;
}

void die_routine(const char *detail, const char *format, ...)
{
	va_list ap;
	static int die_tracker;

	if (++die_tracker > 1)
	{
		fputs("fatal: recursion detected in die handler\n", stderr);
		exit(EXIT_FAILURE);
	}

	va_start(ap, format);
	vreportf("fatal: ", format, ap, detail);

	exit(EXIT_FAILURE);
}

void bug_routine(const char *file, int line, const char *format, ...)
{
	va_list ap;
	char prefix[256];

	va_start(ap, format);
	snprintf(prefix, sizeof(prefix), "BUG: %s:%d: ", file, line);
	vreportf("fatal: ", format, ap, NULL);

	exit(EXIT_FAILURE);
}

const char *msqlite3_pathname = NULL;

static int handle_sqlite3_exec_error(struct sqlite3 *db, va_list ap)
{
	const char *sql, *errmsg;

	sql = va_arg(ap, const char *);
	errmsg = va_arg(ap, const char *);

	return error("Failed to execute '%s' on db '%s'; %s",
			sql, msqlite3_pathname, errmsg);
}

static int handle_sqlite3_prepare_v2_error(struct sqlite3 *db, va_list ap)
{
	const char *sql;

	sql = va_arg(ap, const char *);

	return error_sqlerr(db, "Failed to prepare statement '%s' on "
				  "db '%s'", sql, msqlite3_pathname);
}

static int handle_sqlite3_bind_blob_error(struct sqlite3 *db, va_list ap)
{
	const uint8_t *bin;
	int bin_len;

	char *hex;
	int rescode;

	bin = va_arg(ap, const uint8_t *);
	bin_len = va_arg(ap, int);

	if (bin_len > 16)
	{
		bin_len = 16;
	}
	bin2hex(&hex, xmemdup(bin, bin_len), bin_len);

	rescode = error_sqlerr(db, "Unable to bind blob value '%s%s' on db "
				"'%s'", hex, bin_len > 16 ? "..." : "",
				 msqlite3_pathname);

	free(hex);
	return rescode;
}

static int handle_sqlite3_bind_int64_error(struct sqlite3 *db, va_list ap)
{
	int64_t val;

	val = va_arg(ap, int64_t);

	return error_sqlerr(db, "Unable to bind int64 value '%"PRId64"' on db "
				 "'%s'", val, msqlite3_pathname);
}

static int handle_sqlite3_bind_text_error(struct sqlite3 *db, va_list ap)
{
	const char *val;
	const char *placeholder;

	val = va_arg(ap, const char *);
	placeholder = "";

	if (va_arg(ap, int) > 16)
	{
		placeholder = "...";
	}

	return error_sqlerr(db, "Unable to bind text value '%.16s%s' on db "
				 "'%s'", val, placeholder, msqlite3_pathname);
}

int report_sqlite_error(void *sqlite3_fn, struct sqlite3 *db, ...)
{
	if (msqlite3_pathname == NULL)
	{
		bug("no value given to msqlite3_pathname "
			"before report_sqlite_error()");
	}

	int rescode;

	if (sqlite3_fn == sqlite3_open || sqlite3_fn == sqlite3_open_v2)
	{
		rescode = error_sqlerr(db, "Unable to open db file '%s'",
					msqlite3_pathname);
	}
	else if (sqlite3_fn == sqlite3_key)
	{
		rescode = error("Couldn't apply the key to db '%s'",
				  msqlite3_pathname);
	}
	else if (sqlite3_fn == sqlite3_exec)
	{
		va_list ap;

		va_start(ap, db);
		rescode = handle_sqlite3_exec_error(db, ap);
		va_end(ap);
	}
	else if (sqlite3_fn == sqlite3_avail)
	{
		rescode = error("Encryption key is incorrect or '%s' is not a "
				"valid db file.", msqlite3_pathname);
	}
	else if (sqlite3_fn == sqlite3_prepare_v2)
	{
		va_list ap;

		va_start(ap, db);
		rescode = handle_sqlite3_prepare_v2_error(db, ap);
		va_end(ap);
	}
	else if (sqlite3_fn == sqlite3_bind_blob)
	{
		va_list ap;

		va_start(ap, db);
		rescode = handle_sqlite3_bind_blob_error(db, ap);
		va_end(ap);
	}
	else if (sqlite3_fn == sqlite3_bind_int64)
	{
		va_list ap;

		va_start(ap, db);
		rescode = handle_sqlite3_bind_int64_error(db, ap);
		va_end(ap);
	}
	else if (sqlite3_fn == sqlite3_bind_null)
	{
		rescode = error_sqlerr(db, "Unable to bind null value on db "
					"'%s'", msqlite3_pathname);
	}
	else if (sqlite3_fn == sqlite3_bind_text)
	{
		va_list ap;

		va_start(ap, db);
		rescode = handle_sqlite3_bind_text_error(db, ap);
		va_end(ap);
	}
	else if (sqlite3_fn == sqlite3_step)
	{
		rescode = error_sqlerr(db, "Failed to step statement on db "
					"'%s'", msqlite3_pathname);
	}
	else
	{
		bug("unhandled function pointer %p", sqlite3_fn);
	}

	return rescode;
}

void xio_die(int fd, const char *prefix)
{
	int errnum;

	errnum = errno;
	close(fd);
	errno = errnum;

	if (xiopath == NULL)
	{
		die_errno("%s file '%s'", prefix, xiopath);
	}
	else
	{
		die_errno("%s fd '%d'", prefix, fd);
	}
}
