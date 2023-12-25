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
		{
			return false;
		}
	}
	while (*str);

	return true;
}

bool is_rwx_dir(const char *dirname)
{
	return dirname == NULL ? false : access(dirname, F_OK | R_OK | W_OK | X_OK) == 0;
}

bool is_rw_file(const char *filename)
{
	return filename == NULL ? false : access(filename, F_OK | R_OK | W_OK) == 0;
}

bool is_empty_string(const char *string)
{
	return string == NULL ? true : *string == '\0';
}

char *dirname(const char *filename)
{
	const char *seperator;
	if ((seperator = strrchr(filename, '/')) == NULL || (seperator = strrchr(filename, '\\')) == NULL)
	{
		return NULL;
	}

	char *dirname;
	ptrdiff_t dnlen;

	dnlen = seperator - filename;
	if ((dirname = calloc(dnlen + 1, sizeof(char))) == NULL)
	{
		return NULL;
	}

	memcpy(dirname, filename, dnlen);

	return dirname;
}

char *strpad(size_t padlen)
{
	char *padstr;
	if ((padstr = calloc(padlen + 1, sizeof(char))) == NULL)
	{
		return NULL;
	}

	memset(padstr, ' ', padlen);

	return padstr;
}

int strndup(char **dest, const char *src, size_t n)
{
	if (src == NULL)
	{
		*dest = NULL;
		return EXEC_OK;
	}

	size_t srclen = n < 1 ? strlen(src) + 1 : n + 1;
	if ((*dest = malloc(srclen)) == NULL)
		return MALLOC_FAILURE;

	memcpy(*dest, src, srclen);
	return EXEC_OK;
}

char *strapd(const char *origin, const char *append)
{
	if (origin == NULL || append == NULL)
	{
		return NULL;
	}

	size_t orilen;
	size_t applen;
	char *apdstr;

	orilen = strlen(origin);
	applen = strlen(append);
	if ((apdstr = calloc(orilen + applen + 1, sizeof(char))) == NULL)
	{
		return NULL;
	}

	memcpy(apdstr, origin, orilen);
	memcpy(apdstr + orilen, append, applen);

	return apdstr;
}

char *strsub(const char *src, size_t start, size_t cpylen)
{
	size_t srclen;

	if (src == NULL || (srclen = strlen(src)) < start)
	{
		return NULL;
	}

	if (start + cpylen > srclen)
	{
		cpylen = srclen - start;
	}

	char *substr;
	if ((substr = calloc(cpylen + 1, sizeof(char))) == NULL)
	{
	    return NULL;
	}

	memcpy(substr, src + start, cpylen);

	return substr;
}