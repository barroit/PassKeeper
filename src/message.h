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

#ifndef MESSAGE_H
#define MESSAGE_H

#define ENOTREG -1	/* file is not regular file */

void vreportf(const char *prefix, const char *format, va_list ap, const char *detail);

void note_routine(const char *format, ...)
	__attribute__((format(printf, 1, 2)));

void warning_routine(const char *detail, const char *format, ...)
	__attribute__((format(printf, 2, 3)));

int error_routine(const char *detail, const char *format, ...)
	__attribute__((format(printf, 2, 3)));

void die_routine(const char *detail, const char *format, ...)
	__attribute__((format(printf, 2, 3), noreturn));

void bug_routine(const char *file, int line, const char *format, ...)
	__attribute__((format(printf, 3, 4), noreturn));

#define last_openssl_err()	ERR_reason_error_string(ERR_get_error())

#define note(...)		note_routine(__VA_ARGS__)

#define warning(...)		warning_routine(NULL, __VA_ARGS__)
#define warning_errno(...)	warning_routine(strerror(errno), __VA_ARGS__)

#define error(...)		error_routine(NULL, __VA_ARGS__)
#define error_errno(...)	error_routine(strerror(errno), __VA_ARGS__)
#define error_sqlerr(db, ...)	error_routine(sqlite3_errmsg(db), __VA_ARGS__)
#define error_openssl(...)	error_routine(last_openssl_err(), __VA_ARGS__)

#define die(...)		die_routine(NULL, __VA_ARGS__)
#define die_errno(...)		die_routine(strerror(errno), __VA_ARGS__)
#define die_openssl(...)	die_routine(last_openssl_err(), __VA_ARGS__)

#define bug(...) bug_routine(__FILE__, __LINE__, __VA_ARGS__)

#ifdef WINDOWS_NATIVE
/**
 * same as error_errno except this function set errno to GetLastError()
 */
int error_winerr(const char *format, ...)
	__attribute__((format(printf, 1, 2)));

void die_winerr(const char *format, ...)
	__attribute__((format(printf, 1, 2), noreturn));
#endif

extern const char *msqlite3_pathname;

int report_sqlite_error(void *sqlite3_fn, struct sqlite3 *db, ...);

/**
 * xio_die() print this variable if it's not NULL, otherwise
 * fallback to fd number
 */
extern const char *xiopath;

/**
 * printed message will be ‘__PREFIX__ file/fd xxx’
 */
void xio_die(int fd, const char *prefix)
	__attribute__((noreturn));

#endif /* MESSAGE_H */
