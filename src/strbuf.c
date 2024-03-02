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
#include "misc.h"
#include "debug.h"
#include "rescode.h"

#include <stdarg.h>

#define SB_RESIZE_THRESHOLD	0.8
#define SB_RESIZE_FACTOR	2

#ifdef PK_IS_DEBUG
unsigned sb_resize_count = 0;
#endif

stringbuffer *sballoc(size_t capacity)
{
	stringbuffer *strbuf;
	if ((strbuf = malloc(sizeof(stringbuffer))) == NULL)
	{
		return NULL;
	}

	strbuf->data = malloc(capacity);
	strbuf->size = 0;
	strbuf->capacity = capacity;

	return strbuf;
}

stringbuffer *sbresize(stringbuffer *strbuf, size_t lower_bound)
{
	debug_execute(sb_resize_count++);

	size_t newsize;
	char *newdata;

	newsize = MAX(strbuf->capacity, lower_bound) * SB_RESIZE_FACTOR;
	if ((newdata = realloc(strbuf->data, newsize)) == NULL)
	{
		return NULL;
	}

	strbuf->data = newdata;
	strbuf->capacity = newsize;

	return strbuf;
}

void sbputc(stringbuffer *strbuf, char c)
{
	size_t newsize;

	newsize = strbuf->size + 1;
	if (sb_need_resize(strbuf->capacity, newsize))
	{
		sbresize(strbuf, newsize);
	}

	strbuf->data[strbuf->size++] = c;
	strbuf->data[strbuf->size] = 0;
}

void sbprint(stringbuffer *strbuf, const char *src)
{
	size_t srclen, newsize;

	srclen = strlen(src);
	newsize = strbuf->size + srclen;

	if (sb_need_resize(strbuf->capacity, newsize))
	{
		sbresize(strbuf, newsize);
	}

	memcpy(strbuf->data + strbuf->size, src, srclen + 1); /* contains null-terminator */
	strbuf->size = newsize;
}

void sbprintf(stringbuffer *strbuf, const char *format, ...)
{
	va_list args, _args;
	va_start(args, format);
	va_copy(_args, args);

	size_t srclen, newsize;

	srclen = vsnprintf(NULL, 0, format, _args);
	va_end(_args);

	newsize = strbuf->size + srclen;
	if (sb_need_resize(strbuf->capacity, newsize))
	{
		sbresize(strbuf, newsize);
	}

	vsnprintf(strbuf->data + strbuf->size, srclen + 1, format, args); /* contains null-terminator */
	strbuf->size = newsize;

	va_end(args);
}

void sbnprintf(stringbuffer *strbuf, size_t length, const char *format, ...)
{
	va_list args, _args;
	va_start(args, format);
	va_copy(_args, args);

	size_t char_written, newsize;

	char_written = vsnprintf(NULL, 0, format, _args);
	va_end(_args);

	char_written = char_written > length ? length : char_written;
	newsize = strbuf->size + char_written;

	if (sb_need_resize(strbuf->capacity, newsize))
	{
		sbresize(strbuf, newsize);
	}

	vsnprintf(strbuf->data + strbuf->size, char_written + 1, format, args);
	strbuf->size = newsize;
	va_end(args);
}

void sbfree(stringbuffer *strbuf)
{
	if (strbuf == NULL)
	{
		return;
	}

	free(strbuf->data);
	free(strbuf);
}

bool sb_need_resize(size_t prev, size_t next)
{
	return next + 1 > prev * SB_RESIZE_THRESHOLD; /* +1 for null-terminator */
}