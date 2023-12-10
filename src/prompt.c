#include "prompt.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

int append_string_f(char **str, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	size_t str_length = *str ? strlen(*str) : 0;
	int value_length = vsnprintf(NULL, 0, format, args);

	char *new_str = realloc(*str, str_length + value_length + 1);

	if (new_str == NULL)
	{
		va_end(args);
		return -1;
	}

	*str = new_str;

	vsnprintf(*str + str_length, value_length + 1, format, args);

	va_end(args);
	return 0;
}

int append_string(char **str, const char *value)
{
	if (value == NULL)
		return -1;

	size_t str_length = *str ? strlen(*str) : 0;
	size_t value_length = strlen(value);

	char *new_str = realloc(*str, str_length + value_length + 1);
	if (new_str == NULL)
		return -1;

	*str = new_str;

	strcpy(*str + str_length, value);

	return 0;
}