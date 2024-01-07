#include "strbuffer.h"
#include "utility.h"
#include "debug.h"
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "rescode.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef PK_IS_DEBUG

int strbuffer_resize_count = 0;

#endif /* PK_IS_DEBUG */

string_buffer *sbmake(int capacity)
{
	string_buffer *buf;
	if ((buf = malloc(sizeof(string_buffer))) == NULL)
	{
		return NULL;
	}

	buf->data = malloc(capacity);
	buf->size = 0;
	buf->capacity = capacity;

	return buf;
}

string_buffer *sbresize(string_buffer *buf, int lower_bound)
{
	debug_execute(strbuffer_resize_count++);

	int factor, newsize;

	factor = 2;
	newsize = MAX(buf->capacity, lower_bound) * factor;

	char *newdata;
	if ((newdata = realloc(buf->data, newsize)) == NULL)
	{
		return NULL;
	}

	buf->data = newdata;
	buf->capacity = newsize;

	return buf;
}

void sbputc(string_buffer *buf, char c)
{
	int newsize;
	if ((newsize = buf->size + 1) > buf->capacity * 0.8)
	{
		sbresize(buf, newsize);
	}

	buf->data[buf->size++] = c;
	buf->data[buf->size] = 0;
}

void sbprint(string_buffer *buf, const char *src)
{
	int srclen = strlen(src);
	int newsize = buf->size + srclen;

	if (newsize > buf->capacity * 0.8)
	{
		sbresize(buf, newsize);
	}

	memcpy(buf->data + buf->size, src, srclen + 1);

	buf->size = newsize;
}

void sbprintf(string_buffer *buf, const char *format, ...)
{
	va_list args, _args;
	va_start(args, format);
	va_copy(_args, args);

	int srclen = vsnprintf(NULL, 0, format, _args);
	va_end(_args);
	int newsize = buf->size + srclen;

	if (newsize > buf->capacity * 0.8)
	{
		sbresize(buf, newsize);
	}

	vsnprintf(buf->data + buf->size, srclen + 1, format, args);

	buf->size = newsize;

	va_end(args);
}

void sbnprintf(string_buffer *buf, int len, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	int newsize = buf->size + len;
	if (newsize > buf->capacity * 0.8)
	{
		sbresize(buf, newsize);
	}

	vsnprintf(buf->data + buf->size, len + 1, format, args);

	buf->data[newsize + 1] = '\0';
	buf->size = newsize;

	va_end(args);
}

void sbfree(string_buffer *buf)
{
	free(buf->data);
	free(buf);
}