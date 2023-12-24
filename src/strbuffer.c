#include "strbuffer.h"
#include "utility.h"
#include <string.h>
#include <math.h>
#include <stdarg.h>

#define INIT_CAPACITY 200

int sbinit(struct string_buffer **buf)
{
	if ((*buf = malloc(sizeof(struct string_buffer))) == NULL)
		return MALLOC_FAILURE;

	(*buf)->data = malloc(INIT_CAPACITY);
	(*buf)->size = 0;
	(*buf)->capacity = INIT_CAPACITY;

	return EXEC_OK;
}

int sbresize(struct string_buffer *buf)
{
	int factor = 2;
	int newsize = buf->capacity * factor;
	char *newdata;

	if ((newdata = realloc(buf->data, newsize)) == NULL)
		return MALLOC_FAILURE;

	buf->data = newdata;
	buf->capacity = newsize;

	return EXEC_OK;
}

void sbprint(struct string_buffer *buf, const char *src)
{
	int srclen = strlen(src);
	int newsize = buf->size + srclen;

	if (newsize > buf->capacity * 0.75)
		sbresize(buf);

	memcpy(buf->data + buf->size, src, srclen + 1);

	buf->size = newsize;
}

void sbprintf(struct string_buffer *buf, const char *format, ...)
{
	va_list args, _args;
	va_start(args, format);
	va_copy(_args, args);

	int srclen = vsnprintf(NULL, 0, format, _args);
	va_end(_args);
	int newsize = buf->size + srclen;

	if (newsize > buf->capacity * 0.75)
		sbresize(buf);

	vsnprintf(buf->data + buf->size, srclen + 1, format, args);

	buf->size = newsize;

	va_end(args);
}

int sbclean(struct string_buffer *buf)
{
	free(buf->data);

	if ((buf->data = malloc(INIT_CAPACITY)) == NULL)
		return MALLOC_FAILURE;

	buf->size = 0;
	buf->capacity = INIT_CAPACITY;

	return EXEC_OK;
}

void sbfree(struct string_buffer *buf)
{
	free(buf->data);
	free(buf);
}