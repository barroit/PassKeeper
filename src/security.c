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

#define BLOBKEY_LEN 67
#define KEYSALT_LEN 32

int random_bytes_routine(uint8_t **buf, size_t len, bool alloc_mem)
{
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
		return error_openssl("Failed to generate random bytes");
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

size_t hex2bin(uint8_t **out, char *hex0, size_t hex_len)
{
	uint8_t *bin0, *bin_iter, *bin9;
	size_t bin_len;

	bin_len = hex_len / 2;

	bin0 = (uint8_t *)hex0;
	bin9 = bin0 + bin_len;

	bin_iter = bin0;

	while (bin_iter < bin9)
	{
		*bin_iter = (hexchar2decint(hex0[0]) << 4) |
				hexchar2decint(hex0[1]);

		bin_iter++;
		hex0 += 2;
	}

	*out = bin0;
	return bin_len;
}

size_t bin2hex(char **out, uint8_t *bin0, size_t bin_len)
{
	char *hex_iter, *hex0;
	uint8_t *bin_iter;
	size_t hex_len;

	hex_len = bin_len * 2;

	bin0 = xrealloc(bin0, hex_len + 1);
	bin_iter = bin0 + bin_len - 1;

	hex0 = (char *)bin0;
	hex_iter = hex0 + hex_len - 1;

	while (bin_iter >= bin0)
	{
		*(hex_iter - 0) = decint2hexchar(*bin_iter & 0x0F);
		*(hex_iter - 1) = decint2hexchar(*bin_iter >> 4);

		bin_iter--;
		hex_iter -= 2;
	}

	hex0[hex_len] = 0;

	*out = hex0;
	return hex_len;
}

size_t bin2blob(char **out, uint8_t *bin, size_t bin_len)
{
	char *hex, *blob;
	size_t hex_len, blob_len;

	hex_len = bin2hex(&hex, bin, bin_len);

	blob_len = hex_len + 3;
	blob = xmalloc(blob_len + 1);

	blob[0] = 'x';
	blob[1] = '\'';

	memcpy(blob + 2, hex, hex_len);
	free(hex);

	blob[blob_len - 1] = '\'';
	blob[blob_len] = 0;

	*out = blob;
	return blob_len;
}

size_t blob2bin(uint8_t **out, char *blob, size_t blob_len)
{
	uint8_t *bin;
	size_t bin_len;

	bin_len = hex2bin(&bin, blob + 2, blob_len - 3);
	memmove(blob, bin, bin_len);

	*out = (uint8_t *)blob;
	return bin_len;
}

static bool is_hexstr(const char *str, size_t len)
{
	while (len > 0)
	{
		if (!in_range_i(*str, 'A', 'F') &&
		     !in_range_i(*str, 'a', 'f') &&
		      !isdigit(*str))
		{
			return false;
		}

		str++;
		len--;
	}

	return true;
}

static bool is_saltstr(const char *str, size_t len)
{
	while (len > 0)
	{
		if (*str != '0' && *str != '1')
		{
			return false;
		}

		str++;
		len--;
	}

	return true;
}

bool is_blob_key(const char *key, size_t len)
{
	if (len != BLOBKEY_LEN && len != BLOBKEY_LEN + KEYSALT_LEN)
	{
		return false;
	}

	if (key[0] != 'x' || key[1] != '\'' || key[len - 1] != '\'')
	{
		return false;
	}

	if (!is_hexstr(key + 2, HEXKEY_LEN))
	{
		return false;
	}

	if (len == BLOBKEY_LEN + KEYSALT_LEN)
	{
		if (!is_saltstr(key + 2 + HEXKEY_LEN, KEYSALT_LEN))
		{
			return false;
		}
	}

	return true;
		
}

uint8_t *digest_message_sha256(const uint8_t *message, size_t message_length)
{
	EVP_MD_CTX *mdctx;
	uint8_t *out;

	if ((mdctx = EVP_MD_CTX_new()) == NULL)
	{
		goto failure;
	}

	if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1)
	{
		goto failure;
	}

	if (EVP_DigestUpdate(mdctx, message, message_length) != 1)
	{
		goto failure;
	}

	if ((out = OPENSSL_malloc(EVP_MD_size(EVP_sha256()))) == NULL)
	{
		goto failure;
	}

	if (EVP_DigestFinal_ex(mdctx, out, NULL) != 1)
	{
		goto failure;
	}

	EVP_MD_CTX_free(mdctx);

	return out;

failure:
	die_openssl("An error occured while getting message digest");
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

size_t read_cmdkey(char **key0, const char *message)
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
retry:
	im_print(message);
	if ((len = getline(&key, &cap, stdin)) == -1)
	{
		if (errno != 0)
		{
			die_errno("An error occurred while getting input");
		}

		len = 0;
	}
	else if (len == 1 && *key == '\n')
	{
		im_fputs("\nEmpty keys are not allowed, try again.\n", stderr);
		free(key);

		key = NULL;
		len = 0;

		goto retry;
	}
	else
	{
		while (key[len - 1] == '\n')
		{
			key[--len] = 0;
		}

		*key0 = key;
	}

	if (!no_setattr)
	{
		termios_restore_config(&term);
	}

	return len;
}
