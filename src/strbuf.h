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

#define STRBUF_INIT   { .buf = strbuf_defbuf }
#define STRBUF_INIT_P &(struct strbuf){ .buf = strbuf_defbuf }

void strbuf_destroy(struct strbuf *sb);

void strbuf_printf(struct strbuf *sb, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

void strbuf_putc(struct strbuf *sb, char c);

void strbuf_nprint(struct strbuf *sb, const char *str, size_t sz);

#define strbuf_print(sb, str) strbuf_nprint(sb, str, strlen(str))

#define strbuf_puts(sb, str) strbuf_printf(sb, "%s\n", str)

bool starts_with(const char *str, const char *prefix);

/**
 * Skip `prefix` of the given `str`, `out` contains the rest part
 * of `str`. If the skip failed, `out` remains unchanged
 */
bool skip_prefix(const char *str, const char *prefix, const char **out);

size_t u8strlen(const char *iter);

char *u8substr(const char *src, size_t start, size_t count);

int strtou(const char *str, unsigned *res);

int fprintf_ln(FILE *stream, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#define printf_ln(...) fprintf_ln(stdout, __VA_ARGS__)

/**
 * Replace `c1` with `c2` in `str` inplace.
 */
void replace_char(char *str, char c1, char c2);

#endif /* STRBUF_H */