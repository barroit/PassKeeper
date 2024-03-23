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

#ifndef COMPACT_UTIL_H
#define COMPACT_UTIL_H

#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <inttypes.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <assert.h>

#ifdef POSIX
#define ENV_USERHOME "HOME"
#define DIRSEP "/"
#else
#define ENV_USERHOME "USERPROFILE"
#define DIRSEP "\\"
#endif

#define UNUSED __attribute__((unused))

int error(const char *err, ...) __attribute__((format(printf, 2, 3)));
void die(const char *reason, ...) __attribute__((format(printf, 2, 3)));

void bug_fl(const char *file, int line, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
#define bug(...) bug_fl(__FILE__, __LINE__, __VA_ARGS__)

static inline size_t __attribute__((const)) fixed_growth(size_t sz)
{
	return (sz + 16) * 3 / 2;
}

static inline void *xmalloc(size_t size)
{
	void *ret;
	if ((ret = malloc(size)) == NULL)
	{
		die("out of memory, malloc failed (tried to allocate %"PRIuMAX" bytes)", size);
	}

	return ret;
}

static inline void *xrealloc(void *ptr, size_t size)
{
	if ((ptr = realloc(ptr, size)) == NULL)
	{
		die("out of memory, realloc failed (tried to allocate %"PRIuMAX" bytes)", size);
	}

	return ptr;
}

static inline void *xmemdup(const void *ptr, size_t size)
{
	return memcpy(xmalloc(size), ptr, size);
}

/**
 * copies at most size characters of the stringand make
 * it null-terminated
 */
static inline void *xmemdup_str(const void *ptr, size_t size)
{
	uint8_t *buf;

	buf = xmemdup(ptr, size + 1);
	buf[size] = 0;

	return buf;
}

static inline size_t __attribute__((const)) st_mult(size_t x, size_t y)
{
	if ((SIZE_MAX / x) < y)
	{
		die("size_t overflow: %"PRIuMAX" * %"PRIuMAX, x, y);
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


#ifdef NO_STRCHRNUL
char *pk_strchrnul(const char *s, int c);
#define strchrnul pk_strchrnul
#endif

#ifdef NO_SETENV
int pk_setenv(const char *name, const char *value, int replace);
#define setenv pk_setenv
#endif

#ifdef POSIX
#include <libgen.h>
#endif

#ifdef NO_DIRNAME
char *pk_dirname(char *path);
#define dirname pk_dirname
#endif

#ifdef POSIX
#include <sys/stat.h>
#endif

static inline void xmkdir(const char *path)
{
#if defined(POSIX) || defined(PKTEST) /* for test */
	if (mkdir(path, 0775) != 0)
#else
	if (mkdir(path) != 0)
#endif
	{
		die("failed to create a directory at path '%s'", path);
	}
}

static inline FILE *xfopen(const char *filename, const char *mode)
{
	FILE *fs;
	if ((fs = fopen(filename, mode)) == NULL)
	{
		die("file '%s' reports %s", filename, strerror(errno));
	}

	return fs;
}

#ifdef POSIX
#define test_file_permission(p, s, m) test_file_permission_st(s, m)
#else
#define test_file_permission(p, s, m) test_file_permission_ch(p, m)
#endif

#endif /* COMPACT_UTIL_H */