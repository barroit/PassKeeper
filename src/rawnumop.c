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

byte_t *generate_binkey(size_t length)
{
	byte_t *bin;

	bin = xmalloc(length);

	if (RAND_bytes(bin, length) != 1)
	{
		die("failed to generate binary key");
	}

	return bin;
}

byte_t *hex2bin(const char *hex, size_t size)
{
	byte_t *bin;
	size_t i, ii, bin_size;

	bin_size = size / 2;
	bin = xmalloc(bin_size);

	for (i = 0, ii = 0; ii < bin_size; i += 2, ii++)
	{
		bin[ii] = (hexchar2decnum(hex[i]) << 4) | hexchar2decnum(hex[i + 1]);
	}

	return bin;
}

char *bin2hex(const byte_t *bin, size_t size)
{
	char *hex;
	size_t i, ii, hex_size;

	hex_size = size * 2;
	hex = xmalloc(hex_size + 1);

	for (i = 0, ii = 0; ii < hex_size; i++, ii += 2)
	{
		hex[ii] = decnum2hexchar(bin[i] >> 4);
		hex[ii + 1] = decnum2hexchar(bin[i] & 0x0F);
	}

	hex[hex_size] = 0;
	return hex;
}