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
#include <openssl/rand.h>

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