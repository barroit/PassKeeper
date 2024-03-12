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

static inline void *xmallocs(size_t size)
{
	char *ptr;

	ptr = xmalloc(size + 1);
	ptr[size] = 0;

	return ptr;
}

static inline void *xmemdups(const void *ptr, size_t size)
{
	return memcpy(xmallocs(size), ptr, size);
}

static inline size_t __attribute__((const)) safe_mult(size_t x, size_t y)
{
	if ((SIZE_MAX / x) < y)
	{
		die("size_t overflow: %"PRIuMAX" * %"PRIuMAX, x, y);
	}

	return x * y;
}

#define REALLOC_ARRAY(ptr, size) xrealloc(ptr, safe_mult(sizeof(*ptr), size))

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

#ifdef __WIN64__
static inline char *strchrnul(const char *s, int c)
{
	while (*s && *s != c)
	{
		s++;
	}

	return (char *)s;
}
#endif

#endif /* COMPACT_UTIL_H */