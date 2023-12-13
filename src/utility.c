#include "utility.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

bool is_positive_integer(const char *str)
{
	do
	{
		if (!isdigit(*str++))
			return false;
	}
	while (*str);

	return true;
}

bool is_rw_file(const char *path)
{
	if (path == NULL)
		return false;

	return access(path, F_OK | R_OK | W_OK) == 0;
}

bool is_empty_string(const char *string)
{
	if (string == NULL)
		return true;

	return *string == '\0';
}

int get_space(char **space, int length)
{
	if ((*space = calloc(length + 1, sizeof(char))) == NULL)
		return 1;

	memset(*space, ' ', length);

	return 0;
}