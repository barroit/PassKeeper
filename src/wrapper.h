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

static inline FORCEINLINE void *xmemdup(const void *ptr, size_t size)
{
	return memcpy(xmalloc(size), ptr, size);
}

static inline FORCEINLINE size_t __attribute__((const)) fixed_growth(size_t sz)
{
	return (sz + 16) * 3 / 2;
}

static inline size_t __attribute__((const)) st_mult(size_t x, size_t y)
{
	if ((SIZE_MAX / x) < y)
	{
		die("size_t overflow (%"PRIuMAX" * %"PRIuMAX")", x, y);
	}

	return x * y;
}

#define MOVE_ARRAY(dest, src, size) memmove(dest, src, st_mult(sizeof(*src), size))

#define REALLOC_ARRAY(ptr, size) xrealloc(ptr, st_mult(sizeof(*ptr), size))

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

static inline void xmkdir(const char *path)
{
	if (mkdir(path) != 0)
	{
		die_errno("failed to create a directory at path '%s'", path);
	}
}

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
	return run_sqlite3(sqlite3_db_handle(stmt),
	sqlite3_bind_blob, stmt, idx, val, nr, des);
}

static inline FORCEINLINE int msqlite3_bind_int64(
	struct sqlite3_stmt *stmt, int idx, int64_t val)
{
	return run_sqlite3(sqlite3_db_handle(stmt),
	sqlite3_bind_int64, stmt, idx, val);
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
	return run_sqlite3(sqlite3_db_handle(stmt), sqlite3_bind_text,
			stmt, idx, val, nr, des);
}

static inline FORCEINLINE int msqlite3_step(struct sqlite3_stmt *stmt)
{
	return run_sqlite3(sqlite3_db_handle(stmt), sqlite3_step, stmt);
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
	EXIT_ON_FAILURE(msqlite3_step(stmt__), SQLITE_OK)

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

static inline const char *force_getenv(const char *name)
{
	const char *val;

	if ((val = getenv(name)) == NULL)
	{
		die("Couldn't find the value of env variable '%s'.", name);
	}

	return val;
}

#endif /* WRAPPER_H */
