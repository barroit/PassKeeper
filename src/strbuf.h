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

#ifndef STRBUF_H
#define STRBUF_H

struct strbuf
{
	char *buf;
	size_t length;
	size_t capacity;
};

extern char strbuf_defbuf[];

#define STRBUF_INIT     { .buf = strbuf_defbuf }
#define STRBUF_INIT_PTR &(struct strbuf){ .buf = strbuf_defbuf }

/**
 * free `sb->buf`, after calling this function, the `sb` shall not
 * be used again
 */
static inline FORCEINLINE void strbuf_destroy(struct strbuf *sb)
{
	if (sb->capacity)
	{
		free(sb->buf);
	}
}

/**
 * truncate `sb->buf` to length `0`, but the capacity remains
 * unchanged
 */
static inline FORCEINLINE void strbuf_trunc(struct strbuf *sb)
{
	*sb->buf = 0;
	sb->length = 0;
}

void strbuf_printf(struct strbuf *sb, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

void strbuf_putchar(struct strbuf *sb, char c);

void strbuf_write(struct strbuf *sb, const char *str, size_t sz);

#define strbuf_concat(sb__, str__) strbuf_write(sb__, str__, strlen(str__))

/**
 * DO NOT pass NULL to `str`
 */
void strbuf_puts(struct strbuf *sb, const char *str);

void strbuf_trim_end(struct strbuf *sb);

/**
 * return a copy of `sb->buf`, and truncate `sb`
 */
char *strbuf_detach(struct strbuf *sb);

void strbuf_from_buf(struct strbuf *sb, char *buf);

bool starts_with(const char *str, const char *prefix);

/**
 * Skip `prefix` of the given `str`, `out` contains the rest part
 * of `str`. If the skip failed, `out` remains unchanged
 */
bool skip_prefix(const char *str, const char *prefix, const char **out);

char *concat(const char *str1, const char *str2);

size_t u8strlen(const char *iter);

char *u8substr(const char *src, size_t start, size_t count);

int strtou(const char *str, unsigned *res);

/**
 * Replace `c1` with `c2` in `str` inplace.
 */
void replace_char(char *str, char c1, char c2);

static inline FORCEINLINE bool is_empty_str(const char *str)
{
	return str == NULL || *str == 0;
}

bool is_blank_str(const char *str0);

#endif /* STRBUF_H */
