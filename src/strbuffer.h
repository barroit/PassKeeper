#ifndef STRBUFFER_H
#define STRBUFFER_H

#define BUFFER_OVERFLOW 1

#ifdef DEBUG
extern int resize_execution_count;
#endif /* DEBUG */

struct string_buffer
{
	char *data;
	int size;
	int capacity;
};

int sbinit(struct string_buffer **buf);

int sbresize(struct string_buffer *buf, int lower_bound);

void sbprint(struct string_buffer *buf, const char *src);

void sbprintf(struct string_buffer *buf, const char *format, ...);

void sbnprintf(struct string_buffer *buf, int len, const char *format, ...);

int sbclean(struct string_buffer *buf);

void sbfree(struct string_buffer *buf);

#endif /* STRBUFFER_H */