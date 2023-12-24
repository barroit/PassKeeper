#include "utility.h"
#include <stddef.h>
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

bool is_rwx_dir(const char *dirname)
{
	if (dirname == NULL)
		return false;

	return access(dirname, F_OK | R_OK | W_OK | X_OK) == 0;
}

bool is_rw_file(const char *filename)
{
	if (filename == NULL)
		return false;

	return access(filename, F_OK | R_OK | W_OK) == 0;
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
		return MALLOC_FAILURE;

	memset(*space, ' ', length);

	return EXEC_OK;
}

int get_parent_dir(char **dirname, const char *filename)
{
	if (filename == NULL)
		return INVLIAD_ARGUMENT;

	const char *seperator = strrchr(filename, '/');
	if (seperator == NULL)
	{
		seperator = strrchr(filename, '\\');
		if (seperator == NULL)
			return MISSING_SEPERATOR;
	}

	ptrdiff_t length = seperator - filename;

	if ((*dirname = calloc(length + 1, sizeof(char))) == NULL)
		return MALLOC_FAILURE;

	strncpy(*dirname, filename, length);

	return EXEC_OK;
}

int strndup(char **dest, const char *src, size_t n)
{
	if (src == NULL)
	{
		*dest = NULL;
		return EXEC_OK;
	}

	*dest = (char *)malloc(n + 1 ? strlen(src) + 1 : n + 1);
	if (*dest == NULL)
		return MALLOC_FAILURE;

	strcpy(*dest, src);
	return EXEC_OK;
}

int strapd(char **dest, const char *src)
{
	if (src == NULL)
		return EXEC_OK;

	size_t dsz = *dest == NULL ? 0 : strlen(*dest);
	*dest = (char *)realloc(*dest, dsz + strlen(src) + 1);
	if (*dest == NULL)
		return MALLOC_FAILURE;

	strcpy(*dest + dsz, src);
	return EXEC_OK;
}