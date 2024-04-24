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

void vreportf(const char *prefix, struct report_field *field)
{
	char buffer0[4096], *buffer, *bufptr;
	size_t bufaval;

	buffer = buffer0 + strlen(prefix);
	bufptr = buffer;

	memcpy(buffer0, prefix, buffer - buffer0);
	bufaval = (buffer0 + sizeof(buffer0)) - buffer;

	bufptr += vsnprintf(bufptr, bufaval, field->format, field->ap);

	/* vsnprintf() failure */
	if (bufptr < buffer)
	{
		/**
		 * reset bufptr to buffer, skip the ap
		 * and prepare to write strerror
		 */
		bufptr = buffer;
	}
	else
	{
		bufaval -= bufptr - buffer;
	}

	if (field->strerror && bufaval > 0)
	{
		int nr;
		if ((nr = snprintf(bufptr, bufaval, "; %s", field->strerror)) < 0)
		{
			nr = 0;
		}

		bufptr += nr;
	}

	*bufptr++ = '\n';

	fflush(stderr);
	iwrite(STDERR_FILENO, buffer0, bufptr - buffer0);
}

void note_routine(const char *format, ...)
{
	struct report_field field = {
		.format = format,
	};

	va_start(field.ap, format);
	vreportf("note: ", &field);
	va_end(field.ap);
}

void warning_routine(const char *syserr, const char *format, ...)
{
	struct report_field field = {
		.format = format,
		.strerror = syserr,
	};

	va_start(field.ap, format);
	vreportf("warning: ", &field);
	va_end(field.ap);
}

int error_routine(const char *syserr, const char *format, ...)
{
	struct report_field field = {
		.format = format,
		.strerror = syserr,
	};

	va_start(field.ap, format);
	vreportf("error: ", &field);
	va_end(field.ap);

	return -1;
}

static bool is_dying(void)
{
	static int track;

	return ++track > 1;
}

void die_routine(const char *syserr, const char *format, ...)
{
	if (is_dying())
	{
		fputs("fatal: recursion detected in die handler\n", stderr);
		exit(EXIT_FAILURE);
	}

	struct report_field field = {
		.format = format,
		.strerror = syserr,
	};

	va_start(field.ap, format);
	vreportf("fatal: ", &field);
	exit(EXIT_FAILURE);
}

void bug_routine(const char *file, int line, const char *format, ...)
{
	char prefix[256];
	struct report_field field = {
		.format = format,
	};

	va_start(field.ap, format);
	snprintf(prefix, sizeof(prefix), "BUG: %s:%d: ", file, line);
	vreportf(prefix, &field);

	exit(EXIT_FAILURE);
}

int fprintfln(FILE *stream, const char *fmt, ...)
{
	int nr;
	va_list ap;

	va_start(ap, fmt);
	if ((nr = vfprintf(stream, fmt, ap)) < 0)
	{
		bug("your vfprintf is broken (returned %d)", nr);
	}
	va_end(ap);

	putc('\n', stream);
	nr += 1;
	return nr;
}

const char *msqlite3_pathname = NULL;

int print_sqlite_error(void *sqlite3_fn, struct sqlite3 *db, ...)
{
	if (msqlite3_pathname == NULL)
	{
		bug("no value given to msqlite3_pathname "
			"before print_sqlite_error()");
	}

	if (sqlite3_fn == sqlite3_open || sqlite3_fn == sqlite3_open_v2)
	{
		return error_sqlerr(db, "Unable to open db file '%s'",
					msqlite3_pathname);
	}
	else if (sqlite3_fn == sqlite3_key)
	{
		return error("Couldn't apply the key to db '%s'",
				msqlite3_pathname);
	}
	else if (sqlite3_fn == sqlite3_exec)
	{
		va_list ap;
		const char *sql, *errmsg;

		va_start(ap, db);
		sql = va_arg(ap, const char *);
		errmsg = va_arg(ap, const char *);
		va_end(ap);

		return error("Failed to execute '%s' on db '%s'; "
				"%s", sql, msqlite3_pathname, errmsg);
	}
	else if (sqlite3_fn == sqlite3_avail)
	{
		return error("Encryption key is incorrect or '%s' is not a "
				"valid db file.", msqlite3_pathname);
	}
	else
	{
		bug("unhandled sqlite3 function pointer %p", sqlite3_fn);
	}
}

void report_openssl_error(void)
{
	unsigned long errcode;
	const char *reason;

	errcode = ERR_get_error();
	if (errcode == 0)
	{
		bug("calling report_openssl_error() without "
			"actual error does not make sense");
	}

	reason = ERR_reason_error_string(errcode);

	die("openssl reports '%s'", reason);
}

const char *xio_pathname = NULL;

void xio_die(int fd, const char *prefix)
{
	int errnum;

	errnum = errno;
	close(fd);
	errno = errnum;

	if (xio_pathname)
	{
		die_errno("%s file '%s'", prefix, xio_pathname);
	}
	else
	{
		die_errno("%s fd '%d'", prefix, fd);
	}
}
