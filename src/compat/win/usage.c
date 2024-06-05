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

void warning_winerr(const char *format, ...)
{
	va_list ap;
	DWORD errnum;
	CHAR buf[2048];

	get_winerr_str(buf, sizeof(buf) / sizeof(CHAR));
	errnum = errno;

	va_start(ap, format);
	vreportf("warn: ", format, ap, NULL);
	va_end(ap);

	errno = errnum;
}

int error_winerr(const char *format, ...)
{
	va_list ap;
	DWORD errnum;
	CHAR buf[2048];

	get_winerr_str(buf, sizeof(buf) / sizeof(CHAR));
	errnum = errno;

	va_start(ap, format);
	vreportf("error: ", format, ap, buf);
	va_end(ap);

	errno = errnum;
	return -1;
}

void die_winerr(const char *format, ...)
{
	va_list ap;
	CHAR buf[2048];

	get_winerr_str(buf, sizeof(buf) / sizeof(CHAR));

	va_start(ap, format);
	vreportf("fatal: ", format, ap, buf);
	exit(EXIT_FAILURE);
}
