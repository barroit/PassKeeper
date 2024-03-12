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

#define STRBUF_INIT { .buf = strbuf_defbuf }

void strbuf_alloc(struct strbuf *sb, size_t capacity);

void strbuf_dealloc(struct strbuf *sb);

void strbuf_release(struct strbuf *sb);

void strbuf_reset(struct strbuf *sb);

void strbuf_printf(struct strbuf *sb, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

bool starts_with(const char *str, const char *prefix);

bool skip_prefix(const char *str, const char *prefix, const char **out);

size_t u8strlen(const char *iter);

char *u8substr(const char *src, size_t start, size_t count);

int strtou(const char *str, unsigned *res);

int fprintf_ln(FILE *stream, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#define printf_ln(...) fprintf_ln(stdout, __VA_ARGS__)

#endif /* STRBUF_H */