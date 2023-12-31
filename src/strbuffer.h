#ifndef STRBUFFER_H
#define STRBUFFER_H

#define BUFFER_OVERFLOW 1

#include <stddef.h>

#ifdef PK_IS_DEBUG

extern int resize_execution_count;

#endif /* PK_IS_DEBUG */

typedef struct
{
	char *data;
	int size;
	int capacity;

} string_buffer;

string_buffer *sbmake(int capacity);

string_buffer *sbresize(string_buffer *buf, int lower_bound);

void sbprint(string_buffer *buf, const char *src);

void sbprintf(string_buffer *buf, const char *format, ...);

void sbnprintf(string_buffer *buf, int len, const char *format, ...);

void sbfree(string_buffer *buf);

#endif /* STRBUFFER_H */