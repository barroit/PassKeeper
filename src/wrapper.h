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

#define run_sqlite3(db__, fn__, ...)				\
	do							\
	{							\
		if (fn__(__VA_ARGS__) != SQLITE_OK)		\
		{						\
			return print_sqlite_error(fn__, db);	\
		}						\
		return SQLITE_OK;				\
	}							\
	while (0)

static inline FORCEINLINE int msqlite3_open(const char *filename, struct sqlite3 **db)
{
	run_sqlite3(db, sqlite3_open, filename, db);
}

static inline FORCEINLINE int msqlite3_open_v2(
	const char *filename, struct sqlite3 **db, int flags, const char *vfs)
{
	run_sqlite3(db, sqlite3_open_v2, filename, db, flags, vfs);
}

static inline FORCEINLINE int msqlite3_key(struct sqlite3 *db, const void *key, int sz)
{
	run_sqlite3(db, sqlite3_key, db, key, sz);
}

static inline FORCEINLINE int msqlite3_exec(
	struct sqlite3 *db, const char *sql,
	int (*callback)(void *, int, char **, char **), void *cbargv)
{
	char *errmsg;
	int rescode;

	rescode = SQLITE_OK;
	if (sqlite3_exec(db, sql, callback, cbargv, &errmsg) != SQLITE_OK)
	{
		rescode = print_sqlite_error(sqlite3_exec, db, errmsg);
	}

	sqlite3_free(errmsg);
	return rescode;
}

int sqlite3_avail(struct sqlite3 *db);

static inline FORCEINLINE msqlite3_avail(struct sqlite3 *db)
{
	run_sqlite3(db, sqlite3_avail, db);
}

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

#ifdef SUPPRESS_ACCEPTABLE_LEAKS
void keep_leakref(void *ptr);

#define UNLEAK(ptr__) keep_leakref(ptr__)
#else
#define UNLEAK(ptr__)
#endif


#endif /* WRAPPER_H */
