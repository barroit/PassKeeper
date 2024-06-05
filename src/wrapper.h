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

#ifndef WRAPPER_H
#define WRAPPER_H

char *pk_strerror(int errnum);

#define strerror pk_strerror

#define exit(code) exit(code & 0xFF)

static inline FORCEINLINE void *xmalloc(size_t size)
{
	void *mem;

	if ((mem = malloc(size)) == NULL)
	{
		die("out of memory, malloc failed (tried to allocate "
			"%"PRIuMAX" bytes)", size);
	}

	return mem;
}

static inline FORCEINLINE void *xcalloc(size_t nmemb, size_t size)
{
	void *mem;

	if ((mem = calloc(nmemb, size)) == NULL)
	{
		die("out of memory, malloc failed (tried to allocate "
			"%"PRIuMAX" bytes)", size);
	}

	return mem;
}

static inline FORCEINLINE void *xrealloc(void *ptr, size_t size)
{
	if ((ptr = realloc(ptr, size)) == NULL)
	{
		die("out of memory, realloc failed (tried to allocate "
			"%"PRIuMAX" bytes)", size);
	}

	return ptr;
}

#define xmemdup(ptr, size) memcpy(xmalloc(size), ptr, size)

static inline FORCEINLINE char *xstrdup(const char *s)
{
	char *buf;

	if ((buf = strdup(s)) == NULL)
	{
		die("Out of memory, strdup() failed.");
	}
	
	return buf;
}

int xopen(const char *file, int oflag, ...);

/**
 * we wrap read() and write() by pk_read() and pk_write()
 * these wrappers contain signal and fd blocking handling
 */
ssize_t pk_read(int fd, void *buf, size_t nbytes);
#define read pk_read

ssize_t pk_write(int fd, const void *buf, size_t n);
#define write pk_write

static inline FORCEINLINE ssize_t xwrite(int fd, const void *buf, size_t n)
{
	ssize_t nr;

	if ((nr = write(fd, buf, n)) < 0)
	{
		xio_die(fd, "Unable to write content to");
	}
	else if ((size_t)nr != n)
	{
		xio_die(fd, "Content was truncated when writing to");
	}

	return nr;
}

static inline FORCEINLINE ssize_t xread(int fd, void *buf, size_t nbytes)
{
	ssize_t nr;

	if ((nr = read(fd, buf, nbytes)) == -1)
	{
		xio_die(fd, "Unable to read content from");
	}

	return nr;
}

static inline FORCEINLINE off_t xlseek(int fd, off_t offset, int whence)
{
	off_t ns;

	if ((ns = lseek(fd, offset, whence)) == -1)
	{
		xio_die(fd, "Unable to seek on");
	}

	return ns;
}

#define run_sqlite3(db, fn, ...)\
	( (fn(__VA_ARGS__) != SQLITE_OK) ? report_sqlite_error(fn, db) : SQLITE_OK )

static inline FORCEINLINE int msqlite3_open(const char *filename, struct sqlite3 **db)
{
	return run_sqlite3(*db, sqlite3_open, filename, db);
}

static inline FORCEINLINE int msqlite3_open_v2(
	const char *filename, struct sqlite3 **db, int flags, const char *vfs)
{
	return run_sqlite3(*db, sqlite3_open_v2, filename, db, flags, vfs);
}

static inline FORCEINLINE int msqlite3_key(
	struct sqlite3 *db, const void *key, int sz)
{
	return run_sqlite3(db, sqlite3_key, db, key, sz);
}

int msqlite3_exec(struct sqlite3 *db, const char *sql, int (*callback)(void *, int, char **, char **), void *cbargv, char **errmsg);

int sqlite3_avail(struct sqlite3 *db);

static inline FORCEINLINE int msqlite3_avail(struct sqlite3 *db)
{
	return run_sqlite3(db, sqlite3_avail, db);
}

static inline FORCEINLINE int msqlite3_prepare_v2(
	struct sqlite3 *db, const void *sql, int nr,
	struct sqlite3_stmt **stmt, const char **tail)
{
	return run_sqlite3(db, sqlite3_prepare_v2, db, sql, nr, stmt, tail);
}

static inline FORCEINLINE int msqlite3_bind_blob(
	struct sqlite3_stmt *stmt, int idx,
	const void *val, int nr, void (*des)(void *))
{
	return sqlite3_bind_blob(stmt, idx, val, nr, des) == SQLITE_OK ?
		SQLITE_OK :
		report_sqlite_error(sqlite3_bind_blob, sqlite3_db_handle(stmt),
					val, nr);
}

static inline FORCEINLINE int msqlite3_bind_int64(
	struct sqlite3_stmt *stmt, int idx, int64_t val)
{
	return sqlite3_bind_int64(stmt, idx, val) == SQLITE_OK ?
		SQLITE_OK :
		report_sqlite_error(sqlite3_bind_int64,
					sqlite3_db_handle(stmt), val);
}

static inline FORCEINLINE int msqlite3_bind_null(
	struct sqlite3_stmt *stmt, int idx)
{
	return run_sqlite3(sqlite3_db_handle(stmt), sqlite3_bind_null,
				stmt, idx);
}

static inline FORCEINLINE int msqlite3_bind_text(
	struct sqlite3_stmt *stmt, int idx,
	const void *val, int nr, void (*des)(void *))
{
	return sqlite3_bind_text(stmt, idx, val, nr, des) == SQLITE_OK ?
		SQLITE_OK :
		report_sqlite_error(sqlite3_bind_text, sqlite3_db_handle(stmt),
					val, nr);
}

static inline FORCEINLINE int msqlite3_step(struct sqlite3_stmt *stmt)
{
	return sqlite3_step(stmt) == SQLITE_DONE ?
		SQLITE_DONE :
		report_sqlite_error(sqlite3_step, sqlite3_db_handle(stmt));
}

#define msqlite3_begin_transaction(db)\
	msqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL)

#define msqlite3_end_transaction(db)\
	msqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL)

#define xsqlite3_open(filename, db)\
	require_success(msqlite3_open(filename, db), SQLITE_OK)

#define xsqlite3_open_v2(filename, db, flags, vfs)\
	require_success(msqlite3_open_v2(filename, db, flags, vfs), SQLITE_OK)

#define xsqlite3_key(db, key, len)\
	require_success(msqlite3_key(db, key, len), SQLITE_OK)

#define xsqlite3_exec(db, sql, callback, cbargv, errmsg)\
	require_success(msqlite3_exec(db, sql, callback, cbargv, NULL), SQLITE_OK)

#define xsqlite3_avail(db)\
	require_success(msqlite3_avail(db), SQLITE_OK)

#define xsqlite3_prepare_v2(db, sql, nr, stmt, tail)\
	require_success(msqlite3_prepare_v2(db, sql, nr, stmt, tail), SQLITE_OK)

#define xsqlite3_bind_blob(stmt, idx, val, nr, des)\
	require_success(msqlite3_bind_blob(stmt, idx, val, nr, des), SQLITE_OK)

#define xsqlite3_bind_null(stmt, idx)\
	require_success(msqlite3_bind_null(stmt, idx), SQLITE_OK)

#define xsqlite3_bind_int64(stmt, idx, val)\
	require_success(msqlite3_bind_int64(stmt, idx, val), SQLITE_OK)

#define xsqlite3_bind_text(stmt, idx, val, nr, des)\
	require_success(msqlite3_bind_text(stmt, idx, val, nr, des), SQLITE_OK)

#define xsqlite3_step(stmt)\
	require_success(msqlite3_step(stmt), SQLITE_DONE)

#define xsqlite3_begin_transaction(db)\
	require_success(msqlite3_begin_transaction(db), SQLITE_OK)

#define xsqlite3_end_transaction(db)\
	require_success(msqlite3_end_transaction(db), SQLITE_OK)

#define xsqlite3_bind_or_null(ftype, stmt, idx, val, nr, des)		\
	do								\
	{								\
		if ((val) == NULL)					\
		{							\
			xsqlite3_bind_null(stmt, idx);			\
		}							\
		else							\
		{							\
			xsqlite3_bind_##ftype(stmt, idx, val, nr, des);	\
		}							\
	}								\
	while (0)

#ifdef WINDOWS_NATIVE
HANDLE xCreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

WINBOOL xDuplicateHandle(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle, DWORD dwDesiredAccess, WINBOOL bInheritHandle, DWORD dwOptions);

WINBOOL xSetStdHandle(DWORD nStdHandle, HANDLE hHandle);
#endif

#ifndef DISABLE_SFREE
#define sfree(ptr, len)			\
	do				\
	{				\
		zeromem(ptr, len);	\
		free(ptr);		\
	}				\
	while (0)
#else
#define sfree(ptr, len) free(ptr)
#endif

/**
 * same as fputs except it not use buffer
 *
 * NOTE: msg will be evaluated multiple times
 */
#define im_fputs(str, stream) xwrite(fileno(stream), str, strlen(str))

/**
 * print messages to stdout immediately (without buffer)
 * messages are not automatically appended with newlines
 *
 * NOTE: msg will be evaluated multiple times
 */
#define im_print(str) im_fputs(str, stdout)

/**
 * same as im_print() but only one char will be written
 * into stdout, and c will only be evaluated once
 */
#define im_putchar(c) xwrite(STDOUT_FILENO, &(char){ c }, 1)

#endif /* WRAPPER_H */
