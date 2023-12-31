#include "utility.h"

#include <stdlib.h>
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

bool is_rw_file(const char *pathname)
{
	return pathname == NULL ? false : access(pathname, F_OK | R_OK | W_OK) == 0;
}

bool is_empty_string(const char *string)
{
	return string == NULL ? true : *string == '\0';
}

char *dirof(const char *pathname)
{
	const char *seperator;
	if ((seperator = strrchr(pathname, '/')) == NULL || (seperator = strrchr(pathname, '\\')) == NULL)
	{
		return NULL;
	}

	char *dirname;
	ptrdiff_t dnlen; /* directory name length */

	dnlen = seperator - pathname;
	if ((dirname = calloc(dnlen + 1, sizeof(char))) == NULL)
	{
		return NULL;
	}

	memcpy(dirname, pathname, dnlen);

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

	if (cpylen == 0)
	{
		cpylen = srclen;
	}

	char *substr;
	if ((substr = calloc(cpylen + 1, sizeof(char))) == NULL)
	{
	    return NULL;
	}

	memcpy(substr, src + start, cpylen);

	return substr;
}

#ifdef PK_USE_FHS

#include <sys/stat.h>

#endif

int dirmake(const char *pathname)
{
#ifdef PK_USE_FHS
	return mkdir(pathname, 0755);
#else
	return mkdir(pathname);
#endif
}

#ifdef PK_USE_ARC4RANDOM

#include <bsd/stdlib.h>

#define HAXCHR(chr) ((chr > 9) ? (chr - 10 + 'A') : (chr + '0'))

void *genbytes(size_t length)
{
	unsigned char *buf;
	if ((buf = malloc(length)) == NULL)
	{
		return NULL;
	}

	arc4random_buf(buf, length);

	return (void *)buf;
}

char *btoh(void *data, size_t length)
{
	char *res;
	if ((res = malloc(length * 2 + 1)) == NULL)
	{
		return NULL;
	}

	unsigned char *iter, *tail;

	iter = data;
	tail = iter + length;

	unsigned char hn, ln; /* higher / lower nibble */
	int residx;

	residx = 0;
	while (iter < tail)
	{
		hn = *iter >> 4;
		ln = *iter & 0x0F;

		res[residx++] = HAXCHR(hn);
		res[residx++] = HAXCHR(ln);

		iter++;
	}

	res[residx] = '\0';

	return res;
}

#endif /* PK_USE_ARC4RANDOM */