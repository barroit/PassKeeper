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
