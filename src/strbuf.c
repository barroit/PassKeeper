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

#include "strbuf.h"

static void strbuf_grow(struct strbuf *sb, size_t request_size)
{
	bool factory_new;
	factory_new = sb->capacity == 0;

	if (factory_new)
	{
		sb->buf = NULL;
	}

	CAPACITY_GROW(sb->buf, sb->length + request_size + 1, sb->capacity);

	if (factory_new)
	{
		*sb->buf = 0;
	}
}

char strbuf_defbuf[1];

static inline void strbuf_setlen(struct strbuf *sb, size_t length)
{
	if (sb->buf != strbuf_defbuf)
	{
		sb->buf[length] = 0;
	}

	sb->length = length;
}

static inline void strbuf_addlen(struct strbuf *sb, size_t newlen)
{
	strbuf_setlen(sb, sb->length + newlen);
}

void strbuf_alloc(struct strbuf *sb, size_t capacity)
{
	struct strbuf defsb = STRBUF_INIT;

	memcpy(sb, &defsb, sizeof(struct strbuf));

	if (capacity)
	{
		strbuf_grow(sb, capacity);
	}
}

void strbuf_dealloc(struct strbuf *sb)
{
	if (sb->capacity)
	{
		free(sb->buf);
	}
}

void strbuf_release(struct strbuf *sb)
{
	strbuf_dealloc(sb);
	strbuf_alloc(sb, 0);
}

void strbuf_reset(struct strbuf *sb)
{
	strbuf_setlen(sb, 0);
}

static inline size_t strbuf_size_avail(const struct strbuf *sb)
{
	return sb->capacity == 0 ? 0 : sb->capacity - sb->length - 1;
}

static void strbuf_vprintf(struct strbuf *sb, const char *fmt, va_list ap)
{
	va_list cp;
	int wch;

	if (!strbuf_size_avail(sb))
	{
		strbuf_grow(sb, 64);
	}

	va_copy(cp, ap);
	wch = vsnprintf(sb->buf + sb->length, sb->capacity - sb->length, fmt, cp);
	va_end(cp);

	if (wch < 0)
	{
		bug("your vsnprintf is broken (returned %d)", wch);
	}

	if ((unsigned)wch > strbuf_size_avail(sb))
	{
		strbuf_grow(sb, wch);
		wch = vsnprintf(sb->buf + sb->length, sb->capacity - sb->length, fmt, ap);
	}

	strbuf_addlen(sb, wch);
}

void strbuf_printf(struct strbuf *sb, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	strbuf_vprintf(sb, fmt, ap);

	va_end(ap);
}

bool starts_with(const char *str, const char *prefix)
{
	while (*prefix)
	{
		if (*str++ != *prefix++)
		{
			return false;
		}
	}

	return true;
}

const char *trim_prefix(const char *str, const char *prefix)
{
	do
	{
		if (!*prefix)
		{
			return str;
		}
	}
	while (*str++ == *prefix++);

	return NULL;
}

char *concat(const char *str1, const char *str2)
{
	size_t destlen, len1, len2;
	char *dest;

	len1 = strlen(str1);
	len2 = strlen(str2);
	destlen = len1 + len2;

	dest = xmalloc(destlen + 1);

	memcpy(dest, str1, len1);
	memcpy(dest + len1, str2, len2);
	dest[destlen] = 0;

	return dest;
}

static inline bool is_start_byte(const char c)
{
	return (c & 0xC0) != 0x80;
}

size_t u8strlen(const char *iter)
{
	size_t length;

	length = 0;
	while (*iter)
	{
		if (is_start_byte(*iter++))
		{
			length++;
		}
	}

	return length;
}

char *u8substr(const char *iter, size_t start_index, size_t substr_length)
{
	const char *head;
	size_t char_count;
	bool unknow_length, finish_calc;

	head = NULL;
	char_count = 0;
	unknow_length = substr_length == 0;
	finish_calc = false;

	while (*iter)
	{
		if (!is_start_byte(*iter))
		{
			iter++;
			continue;
		}

		if (finish_calc)
		{
			break;
		}

		iter++;
		char_count++;

		/*
		 * skip to next char until the current 'character'
		 * index reaches the start_index
		 * due to the flow, we need char_count - 1 for
		 * the current 'character' index
		 */
		if (char_count - 1 < start_index)
		{
			continue;
		}

		if (char_count - 1 == start_index)
		{
			head = iter - 1;
		}

		if (unknow_length)
		{
			continue; /* let iter goes to the end */
		}

		substr_length--;

		if (substr_length == 0)
		{
			finish_calc = true;
		}
	}

	if (head == NULL)
	{
		bug("u8substr() has an out-of-size start_index");
	}
 
	char *dest;
	ptrdiff_t destsz;

	destsz = iter - head;
	dest = xmalloc(destsz + 1);

	memcpy(dest, head, destsz);
	dest[destsz] = 0;

	return dest;
}

int strtou(const char *str, unsigned *res)
{
	if (*str == '-')
	{
		return EINVAL;
	}

	if (*str == 0)
	{
		return EILSEQ;
	}

	char *end;
	unsigned long int tmpres;

	errno = 0;
	tmpres = strtoul(str, &end, 10);

	if (*end != 0)
	{
		return EILSEQ;
	}

	if (errno == ERANGE || tmpres > UINT_MAX)
	{
		return ERANGE;
	}

	*res = (unsigned)tmpres;

	return 0;
}

int fprintf_ln(FILE *stream, const char *fmt, ...)
{
	int wch;
	va_list ap;

	va_start(ap, fmt);
	wch = vfprintf(stream, fmt, ap);
	if (wch < 0)
	{
		bug("your vfprintf is broken (returned %d)", wch);
	}

	putc('\n', stream);
	va_end(ap);

	return wch;
}