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

static LPSTR get_winerr_str(LPSTR buf, DWORD bufsz)
{
	DWORD errnum;
	DWORD flags, langid;

	errnum = GetLastError();
	flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	langid = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

	FormatMessage(flags, NULL, errnum, langid, buf, bufsz, NULL);

	errno = errnum;
	return buf;
}

static void winerr_routine(const char *format, struct report_field *field, int *errnum)
{
	CHAR buf[2048];

	get_winerr_str(buf, sizeof(buf) / sizeof(CHAR));
	if (errnum != NULL)
	{
		*errnum = errno;
	}

	const struct report_field field0 = {
		.format = format,
		.strerror = buf,
	};

	memcpy(field, &field0, sizeof(struct report_field));
}

void warn_winerr(const char *format, ...)
{
	struct report_field field;
	int errnum;

	winerr_routine(format, &field, &errnum);

	va_start(field.ap, format);
	vreportf("warn: ", &field);
	va_end(field.ap);

	errno = errnum;
}

int error_winerr(const char *format, ...)
{
	struct report_field field;
	int errnum;

	winerr_routine(format, &field, &errnum);

	va_start(field.ap, format);
	vreportf("error: ", &field);
	va_end(field.ap);

	errno = errnum;
	return -1;
}

void die_winerr(const char *format, ...)
{
	struct report_field field;

	winerr_routine(format, &field, NULL);

	va_start(field.ap, format);
	vreportf("fatal: ", &field);
	exit(EXIT_FAILURE);
}