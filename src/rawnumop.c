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

#include "rawnumop.h"

uint8_t *random_bytes(size_t length)
{
	uint8_t *bin;

	bin = xmalloc(length);

	if (RAND_bytes(bin, length) != 1)
	{
		die("failed to generate binary key");
	}

	return bin;
}

uint8_t *hex2bin(char *hex, size_t hexsz)
{
	uint8_t *binhead, *binit, *bintail;

	binhead = (uint8_t *)hex;
	binit = binhead;
	bintail = binhead + (hexsz / 2);
	while (binit < bintail)
	{
		*binit = (hexchar2decnum(hex[0]) << 4) | hexchar2decnum(hex[1]);

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
		*(hexit - 0) = decnum2hexchar(*binit & 0x0F);
		*(hexit - 1) = decnum2hexchar(*binit >> 4);
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

	/**
	 * upper camel case with no x version api
	 * ugly and sad :(
	 */
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
