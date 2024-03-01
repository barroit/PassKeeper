#include "encrypt.h"
#include "misc.h"
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>

void *get_binary_key(size_t length)
{
	unsigned char *buf;
	if ((buf = malloc(length)) == NULL)
	{
		return NULL;
	}

	if (RAND_bytes(buf, length) != 1)
	{
		return NULL;
	}

	return buf;
}

char *bin_to_hex(void *data, size_t length)
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

		res[residx++] = byte_to_hexchar(hn);
		res[residx++] = byte_to_hexchar(ln);

		iter++;
	}

	res[residx] = '\0';

	return res;
}

void *hex_to_bin(const char *str, size_t *size)
{
	size_t strsz, ressz;
	if ((strsz = strlen(str)) % 2 != 0) /* invalid hex str */
	{
		return NULL;
	}

	ressz = strsz / 2;

	unsigned char *res, *resiter;
	if ((res = malloc(ressz)) == NULL)
	{
		return NULL;
	}

	const char *strtail, *striter;

	striter = str;
	strtail = str + strsz;
	resiter = res;
	while (striter < strtail)
	{
		if (!is_hexchr(*striter) || !is_hexchr(*(striter + 1)))
		{
			free(res);
			return NULL;
		}

		*resiter = hexchar_to_byte(*striter) << 4;
		*resiter += hexchar_to_byte(*(striter + 1));

		resiter++;
		striter += 2;
	}

	if (size != NULL)
	{
		*size = ressz;
	}

	return res;
}

char byte_to_hexchar(unsigned char c)
{
	return c > 9 ? c - 10 + 'A' : c + '0';
}

unsigned char hexchar_to_byte(char c)
{
	unsigned char v1, v2, v3;

	v1 = IN_RANGE(c, 'a', 'f') ? c - 'a' + 10 : 0;
	v2 = IN_RANGE(c, 'A', 'F') ? c - 'A' + 10 : 0;
	v3 = IN_RANGE(c, '0', '9') ? c - '0' : 0;

	return v1 + v2 + v3;
}