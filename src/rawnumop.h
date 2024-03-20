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

#ifndef RAWNUMOP_H
#define RAWNUMOP_H

/**
 * Check if `x` is greater than (or equal to) `r1` and less
 * than (or equal to) `r2`
*/
static inline bool in_range(int x, int r1, int r2, bool inclusive)
{
	return x > (r1 - inclusive) && x < (r2 + inclusive);
}

static inline byte_t hexchar2decnum(char c)
{
	return isupper(c) ? (c - 'A' + 10) :
		islower(c) ? (c - 'a' + 10) :
		 (c - '0');
}

static inline char decnum2hexchar(byte_t n)
{
	return n < 10 ? (n + '0') : (n - 10 + 'A');
}

byte_t *generate_binkey(size_t length);

/**
 * convert at most `size` characters of the `hex`
 * to binary, `size` must be a multiple of 2
 */
byte_t *hex2bin(const char *hex, size_t size);

/**
 * convert at most `size` bytes of the `bin`
 * to hex characters
 */
char *bin2hex(const byte_t *bin, size_t size);

#endif /* RAWNUMOP_H */