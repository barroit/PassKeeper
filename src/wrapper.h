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

#define FLEX_ARRAY

#define exit(code__) exit((code__) & 0xff)

#define is_pow2(x__)\
	( (x__) != 0 && ( (x__) & ( (x__) - 1 ) ) == 0 )

/**
 * check if `x__` is in range `r1` and `r2` exclusive
 */
#define in_range_e(x__, r1__, r2__)\
	( (x__) > (r1__) && (x__) < (r2__) )

/**
 * check if `x__` is in range `r1` and `r2` inclusive
 */
#define in_range_i(x__, r1__, r2__)\
	( (x__) >= (r1__) && (x__) <= (r2__) )

static inline void *xmalloc(size_t size)
{
	void *ret;
	if ((ret = malloc(size)) == NULL)
	{
		die("out of memory, malloc failed (tried to allocate "
			"%"PRIuMAX" bytes)", size);
	}

	return ret;
}

static inline void *xrealloc(void *ptr, size_t size)
{
	if ((ptr = realloc(ptr, size)) == NULL)
	{
		die("out of memory, realloc failed (tried to allocate "
			"%"PRIuMAX" bytes)", size);
	}

	return ptr;
}

#define xmemdup(p__, l__) memcpy(xmalloc(l__), (p__), (l__))

static inline size_t __attribute__((const)) st_add(size_t x, size_t y)
{
	if ((SIZE_MAX - x) < y)
	{
		die("size_t overflow (%"PRIuMAX" + %"PRIuMAX")", x, y);
	}

	return x + y;
}

#define st_add3(a__, b__, c__) st_add(st_add((a__), (b__)), (c__))

static inline size_t __attribute__((const)) st_mult(size_t x, size_t y)
{
	if ((SIZE_MAX / x) < y)
	{
		die("size_t overflow (%"PRIuMAX" * %"PRIuMAX")", x, y);
	}

	return x * y;
}

#define fixed_growth(l__) ( st_mult(st_add((l__), 16), 3) / 2 )

#define MOVE_ARRAY(dest, src, size) memmove(dest, src, st_mult(sizeof(*src), size))

#define REALLOC_ARRAY(ptr, size) xrealloc(ptr, st_mult(sizeof(*ptr), size))

#define FLEX_ALLOC_ARRAY(obj__, field__, buf__, len__)			\
	do								\
	{								\
		(obj__) = xmalloc(st_add3(sizeof(*(obj__)), len__, 1));	\
		memcpy((obj__)->field__, (buf__), len__);		\
	}								\
	while (0)

#define CAPACITY_GROW(ptr, size, cap)			\
	do						\
	{						\
		if (size > cap)				\
		{					\
			cap = fixed_growth(cap);	\
			cap = cap < size ? size : cap;	\
			ptr = REALLOC_ARRAY(ptr, cap);	\
		}					\
	}						\
	while (0)

int xopen(const char *file, int oflag, ...);

ssize_t iread(int fd, void *buf, size_t nbytes);

/**
 * function prefixed by 'i' are safe to use in child process
 */
ssize_t iwrite(int fd, const void *buf, size_t n);

static inline ssize_t xwrite(int fd, const void *buf, size_t n)
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

static inline ssize_t xread(int fd, void *buf, size_t nbytes)
{
	ssize_t nr;

	if ((nr = read(fd, buf, nbytes)) == -1)
	{
		xio_die(fd, "Unable to read content from");
	}

	return nr;
}

static inline off_t xlseek(int fd, off_t offset, int whence)
{
	off_t ns;

	if ((ns = lseek(fd, offset, whence)) == -1)
	{
		xio_die(fd, "Unable to seek on");
	}

	return ns;
}

#define EXIT_ON_FAILURE(lc__, rc__)			\
	do						\
	{						\
		if ((lc__) != (rc__))			\
		{					\
			exit(EXIT_FAILURE);		\
		}					\
	}						\
	while (0)

#define run_sqlite3(db__, fn__, ...)\
	((fn__(__VA_ARGS__) != SQLITE_OK) ? print_sqlite_error(fn__, db__) : SQLITE_OK)

static inline int msqlite3_open(const char *filename, struct sqlite3 **db)
{
	return run_sqlite3(db, sqlite3_open, filename, db);
}

static inline FORCEINLINE int msqlite3_open_v2(
	const char *filename, struct sqlite3 **db, int flags, const char *vfs)
{
	return run_sqlite3(db, sqlite3_open_v2, filename, db, flags, vfs);
}

static inline FORCEINLINE int msqlite3_key(
	struct sqlite3 *db, const void *key, int sz)
{
	return run_sqlite3(db, sqlite3_key, db, key, sz);
}

int msqlite3_exec(struct sqlite3 *db, const char *sql, int (*callback)(void *, int, char **, char **), void *cbargv, char **errmsg);

int sqlite3_avail(struct sqlite3 *db);

static inline FORCEINLINE msqlite3_avail(struct sqlite3 *db)
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
		print_sqlite_error(sqlite3_bind_blob, sqlite3_db_handle(stmt),
					val, nr);
}

static inline FORCEINLINE int msqlite3_bind_int64(
	struct sqlite3_stmt *stmt, int idx, int64_t val)
{
	return sqlite3_bind_int64(stmt, idx, val) == SQLITE_OK ?
		SQLITE_OK :
		print_sqlite_error(sqlite3_bind_int64,
					sqlite3_db_handle(stmt), val);
}

static inline FORCEINLINE int msqlite3_bind_null(
	struct sqlite3_stmt *stmt, int idx)
{
	return run_sqlite3(sqlite3_db_handle(stmt),
	sqlite3_bind_null, stmt, idx);
}

static inline FORCEINLINE int msqlite3_bind_text(
	struct sqlite3_stmt *stmt, int idx,
	const void *val, int nr, void (*des)(void *))
{
	return sqlite3_bind_text(stmt, idx, val, nr, des) == SQLITE_OK ?
		SQLITE_OK :
		print_sqlite_error(sqlite3_bind_text, sqlite3_db_handle(stmt),
					val, nr);
}

static inline FORCEINLINE int msqlite3_step(struct sqlite3_stmt *stmt)
{
	return sqlite3_step(stmt) == SQLITE_DONE ?
		SQLITE_DONE :
		print_sqlite_error(sqlite3_step, sqlite3_db_handle(stmt));
}

#define msqlite3_begin_transaction(db__)\
	msqlite3_exec(db__, "BEGIN TRANSACTION;", NULL, NULL, NULL)

#define msqlite3_end_transaction(db__)\
	msqlite3_exec(db__, "END TRANSACTION;", NULL, NULL, NULL)

#define xsqlite3_open(filename__, db__)\
	EXIT_ON_FAILURE(msqlite3_open(filename__, db__), SQLITE_OK)

#define xsqlite3_open_v2(filename__, db__, flags__, vfs__)\
	EXIT_ON_FAILURE(msqlite3_open_v2(filename__, db__, flags__, vfs__), SQLITE_OK)

#define xsqlite3_key(db__, key__, sz__)\
	EXIT_ON_FAILURE(msqlite3_key(db__, key__, sz__), SQLITE_OK)

#define xsqlite3_exec(db__, sql__, callback__, cbargv__, errmsg__)\
	EXIT_ON_FAILURE(msqlite3_exec(db__, sql__, callback__, cbargv__, NULL), SQLITE_OK)

#define xsqlite3_avail(db__)\
	EXIT_ON_FAILURE(msqlite3_avail(db__), SQLITE_OK)

#define xsqlite3_prepare_v2(db__, sql__, nr__, stmt__, tail__)\
	EXIT_ON_FAILURE(msqlite3_prepare_v2(db__, sql__, nr__, stmt__, tail__), SQLITE_OK)

#define xsqlite3_bind_blob(stmt__, idx__, val__, nr__, des__)\
	EXIT_ON_FAILURE(msqlite3_bind_blob(stmt__, idx__, val__, nr__, des__), SQLITE_OK)

#define xsqlite3_bind_null(stmt__, idx__)\
	EXIT_ON_FAILURE(msqlite3_bind_null(stmt__, idx__), SQLITE_OK)

#define xsqlite3_bind_int64(stmt__, idx__, val__)\
	EXIT_ON_FAILURE(msqlite3_bind_int64(stmt__, idx__, val__), SQLITE_OK)

#define xsqlite3_bind_text(stmt__, idx__, val__, nr__, des__)\
	EXIT_ON_FAILURE(msqlite3_bind_text(stmt__, idx__, val__, nr__, des__), SQLITE_OK)

#define xsqlite3_step(stmt__)\
	EXIT_ON_FAILURE(msqlite3_step(stmt__), SQLITE_DONE)

#define xsqlite3_begin_transaction(db__)\
	EXIT_ON_FAILURE(msqlite3_begin_transaction(db__), SQLITE_OK)

#define xsqlite3_end_transaction(db__)\
	EXIT_ON_FAILURE(msqlite3_end_transaction(db__), SQLITE_OK)

#define xsqlite3_bind_or_null(ftype__, stmt__, idx__, val__, nr__, des__)	\
	do									\
	{									\
		if ((val__) == NULL)						\
		{								\
			xsqlite3_bind_null(stmt__, idx__);			\
		}								\
		else								\
		{								\
			xsqlite3_bind_##ftype__(stmt__, idx__, val__,		\
						nr__, des__);			\
		}								\
	}									\
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
		zeromem((ptr), (len));	\
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
#define im_fputs(str, stream) xwrite(fileno(stream), (str), strlen(str))

/**
 * print messages to stdout immediately (without buffer)
 * messages are not automatically appended with newlines
 *
 * NOTE: msg will be evaluated multiple times
 */
#define im_print(str) im_fputs((str), stdout)

/**
 * same as im_print() but only one char will be written
 * into stdout, and c will only be evaluated once
 */
#define im_putchar(c) xwrite(STDOUT_FILENO, &(char){ (c) }, 1)

#endif /* WRAPPER_H */
