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

char strbuf_defbuf[1];

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

static inline void strbuf_setlen(struct strbuf *sb, size_t length)
{
	if (sb->buf != strbuf_defbuf)
	{
		sb->buf[length] = 0;
	}

	sb->length = length;
}

static inline FORCEINLINE void strbuf_addlen(struct strbuf *sb, size_t newlen)
{
	strbuf_setlen(sb, sb->length + newlen);
}

static inline FORCEINLINE size_t strbuf_size_avail(const struct strbuf *sb)
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

void strbuf_putchar(struct strbuf *sb, char c)
{
	if (!strbuf_size_avail(sb))
	{
		strbuf_grow(sb, 1);
	}

	sb->buf[sb->length++] = c;
	sb->buf[sb->length] = 0;
}

void strbuf_write(struct strbuf *sb, const char *str, size_t sz)
{
	if (sz > strbuf_size_avail(sb))
	{
		strbuf_grow(sb, sz);
	}

	memcpy(sb->buf + sb->length, str, sz);

	strbuf_addlen(sb, sz);
}

void strbuf_puts(struct strbuf *sb, const char *str)
{
	size_t size;
	char *bufptr;

	size = strlen(str) + 1; /* add one for newline char */
	if (size > strbuf_size_avail(sb))
	{
		strbuf_grow(sb, size);
	}

	bufptr = sb->buf;

	memcpy(bufptr += sb->length, str, size - 1);
	bufptr[size - 1] = '\n';

	strbuf_addlen(sb, size);
}

void strbuf_trim_end(struct strbuf *sb)
{
	for (; sb->length > 0 && isspace(sb->buf[sb->length - 1]); sb->length--);
	sb->buf[sb->length] = 0;
}

char *strbuf_detach(struct strbuf *sb)
{
	char *bufcopy;

	bufcopy = malloc(sb->length + 1); /* add one for null terminator */
	memcpy(bufcopy, sb->buf, sb->length + 1);
	strbuf_trunc(sb);

	return bufcopy;
}

void strbuf_from_buf(struct strbuf *sb, char *buf)
{
	if (sb->buf != NULL)
	{
		bug("calling strbuf_from_buf() on a non empty strbuf");
	}

	if (buf != NULL)
	{
		sb->buf = buf;
		sb->length = strlen(buf);
		sb->capacity = sb->length + 1;

		strbuf_grow(sb, 64);
	}
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

bool skip_prefix(const char *str, const char *prefix, const char **out)
{
	do
	{
		if (!*prefix)
		{
			*out = str;
			return true;
		}
	}
	while (*str++ == *prefix++);

	return false;
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
		 * skip to next char until the current ‘character’
		 * index reaches the start_index
		 * due to the flow, we need char_count - 1 for
		 * the current ‘character’ index
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

int strtou(const char *str, unsigned *res0)
{
	if (*str == '-')
	{
		errno = EINVAL;
		return -1;
	}

	char *end;
	unsigned long res;

	errno = 0;
	res = strtoul(str, &end, 10);

	if (errno != 0)
	{
		return -1;
	}

	if (*end != 0)
	{
		errno = EILSEQ;
		return -1;
	}

	if (res > UINT_MAX)
	{
		errno = ERANGE;
		return -1;
	}

	*res0 = (unsigned)res;

	return 0;
}

void replace_char(char *str, char c1, char c2)
{
	while (*str)
	{
		if (*str == c1)
		{
			*str = c2;
		}

		str++;
	}
}

bool is_blank_str(const char *str0)
{
	if (is_empty_str(str0))
	{
		return true;
	}

	const char *str;

	str = str0;
	str0 += strlen(str0);
	while (str != str0 && isspace(*str))
	{
		str++;
	}

	return str == str0;
}
