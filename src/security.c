/****************************************************************************
**
** Copyright 2023, 2024 Jiamu Sun
** Contact: barroit@linux.com
**
** This file is part of PassKeeper.
**
** PassKeeper is free software: you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation, either version 3 of the License, or (at your
** option) any later version.
**
** PassKeeper is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with PassKeeper. If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "security.h"

int random_bytes_routine(uint8_t **buf, size_t len, bool alloc_mem)
{
	if (buf == NULL)
	{
		bug("buf shall not be NULL");
	}

	if (alloc_mem)
	{
		*buf = xmalloc(len);
	}

	if (RAND_bytes(*buf, len) != 1) /* 1 for success */
	{
		if (alloc_mem)
		{
			free(*buf);
		}

		*buf = NULL;
		return 1;
	}

	return 0;
}

/**
 * convert a hex char to decimal integer
 */
#define hexchar2decint(c__)				\
	(						\
	  isupper(c__) ? ( (c__) - 'A' + 10 ) :		\
	   islower(c__) ? ( (c__) - 'a' + 10 ) :	\
	    ( (c__) - '0' )				\
	)

/**
 * opposite of hexchar2decint()
 */
#define decint2hexchar(n__)\
	( (n__) < 10 ? ( (n__) + '0' ) : ( (n__) - 10 + 'A' ) )

uint8_t *hex2bin(char *hex, size_t hexsz)
{
	uint8_t *binhead, *binit, *bintail;

	binhead = (uint8_t *)hex;
	binit = binhead;
	bintail = binhead + (hexsz / 2);
	while (binit < bintail)
	{
		*binit = (hexchar2decint(hex[0]) << 4) | hexchar2decint(hex[1]);

		binit++;
		hex += 2;
	}

	return binhead;
}

char *bin2hex(uint8_t *bin, size_t binsz)
{
	char *hexit, *hexhead;
	uint8_t *binit;
	size_t hexsz;

	hexsz = binsz * 2;
	bin = xrealloc(bin, hexsz + 1);
	binit = bin + binsz - 1;
	hexhead = (char *)bin;
	hexit = hexhead + hexsz - 1;

	while (binit >= bin)
	{
		*(hexit - 0) = hexchar2decint(*binit & 0x0F);
		*(hexit - 1) = hexchar2decint(*binit >> 4);
		binit--;
		hexit -= 2;
	}

	hexhead[hexsz] = 0;
	return hexhead;
}

char *bin2blob(uint8_t *binkey, size_t binlen)
{
	char *hexkey, *blobkey;
	size_t bloblen;

	hexkey  = bin2hex(binkey, binlen);
	bloblen = binlen * 2 + 3;

	blobkey = xmalloc(bloblen + 1);
	snprintf(blobkey, bloblen + 1, "x'%s'", hexkey);

	free(hexkey);
	return blobkey;
}

#define is_hexchar(c__)\
	in_range_i(c__, 'A', 'F') || in_range_i(c__, 'a', 'f') || isdigit(c__)

bool is_hexstr(const char *hex, size_t size)
{
	while (size > 0)
	{
		if (!is_hexchar(*hex++))
		{
			return false;
		}

		size--;
	}

	return true;
}

bool is_saltstr(const char *salt, size_t size)
{
	while (size > 0)
	{
		if (*salt != '0' && *salt != '1')
		{
			return false;
		}

		salt++;
		size--;
	}

	return true;
}

uint8_t *digest_message_sha256(const uint8_t *message, size_t message_length)
{
	EVP_MD_CTX *mdctx;
	uint8_t *out;

	if ((mdctx = EVP_MD_CTX_new()) == NULL)
	{
		report_openssl_error();
	}

	if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1)
	{
		report_openssl_error();
	}

	if (EVP_DigestUpdate(mdctx, message, message_length) != 1)
	{
		report_openssl_error();
	}

	if ((out = OPENSSL_malloc(EVP_MD_size(EVP_sha256()))) == NULL)
	{
		report_openssl_error();
	}

	if (EVP_DigestFinal_ex(mdctx, out, NULL) != 1)
	{
		report_openssl_error();
	}

	EVP_MD_CTX_free(mdctx);

	return out;
}

int verify_digest_sha256(
	const uint8_t *message, size_t message_length,
	const uint8_t *prev_digest)
{
	int rescode;
	uint8_t *next_digest;

	next_digest = digest_message_sha256(message, message_length);
	rescode = memcmp(next_digest, prev_digest, EVP_MD_size(EVP_sha256()));

	clean_digest(next_digest);

	return rescode;
}

void secure_destroy(void *ptr, size_t len)
{
	zeromem(ptr, len);
	free(ptr);
}

size_t read_cmdkey(char **key0)
{
	struct termios term;
	bool no_setattr;

	no_setattr = false;
	if (termios_disable_echo(&term) == -1)
	{
		no_setattr = true;
	}

	char *key;
	ssize_t len;
	size_t cap;

	key = NULL;
	cap = 0;

	errno = 0;
	if ((len = getline(&key, &cap, stdin)) == -1)
	{
		if (errno != 0)
		{
			warning_errno("An error occurred while getting input");
		}

		len = 0;
	}
	else
	{
		if (key[len - 1] == '\n')
		{
			key[len - 1] = 0;
		}

		*key0 = key;
	}

	if (!no_setattr)
	{
		termios_restore_config(&term);
	}

	return len;
}
