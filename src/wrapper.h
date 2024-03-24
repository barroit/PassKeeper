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

int error(const char *err, ...) __attribute__((format(printf, 2, 3)));
void die(const char *reason, ...) __attribute__((format(printf, 2, 3)));

void bug_fl(const char *file, int line, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
#define bug(...) bug_fl(__FILE__, __LINE__, __VA_ARGS__)

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

static inline size_t __attribute__((const)) fixed_growth(size_t sz)
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

static inline size_t xfwrite(const void *ptr, size_t size, size_t n, FILE *s)
{
	size_t nn;

	if ((nn = fwrite(ptr, size, n, s)) != n)
	{
		die("expected to write '%lu' objects to file, but actually wrote '%lu'", nn, n);
	}

	return nn;
}

static inline int msqlite3_open(const char *db_path, struct sqlite3 **db)
{
	if (sqlite3_open(db_path, db))
	{
		error("cannot open database, %s", sqlite3_errmsg(*db));
		return 1;
	}

	return 0;
}

static inline int msqlite3_key(struct sqlite3 *db, const void *key, int sz)
{
	if (sqlite3_key(db, key, sz))
	{
		error("cannot apply key to database");
		return 1;
	}

	return 0;
}

static inline int msqlite3_exec(struct sqlite3 *db, const char *sql)
{
	char *errmsg;
	if (sqlite3_exec(db, sql, NULL, NULL, &errmsg))
	{
		error("cannot evaluate sql statements, %s", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	return 0;
}

#endif /* WRAPPER_H */