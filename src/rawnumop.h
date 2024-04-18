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

static inline bool is_pow2(unsigned x)
{
	return x != 0 && (x & (x - 1)) == 0;
}

/**
 * Check if `x` is greater than (or equal to) `r1` and less
 * than (or equal to) `r2`
*/
static inline bool in_range(int x, int r1, int r2, bool inclusive)
{
	return x > (r1 - inclusive) && x < (r2 + inclusive);
}

static inline bool in_range_u(unsigned x, unsigned r1, unsigned r2, bool inclusive)
{
	return x > (r1 - inclusive) && x < (r2 + inclusive);
}

static inline uint8_t hexchar2decnum(char c)
{
	return isupper(c) ? (c - 'A' + 10) :
		islower(c) ? (c - 'a' + 10) :
		 (c - '0');
}

static inline bool is_hexchar(char c)
{
	return in_range(c, 'A', 'F', 1) || in_range(c, 'a', 'f', 1) || isdigit(c);
}

static inline char decnum2hexchar(uint8_t n)
{
	return n < 10 ? (n + '0') : (n - 10 + 'A');
}

uint8_t *random_bytes(size_t length);

/**
 * convert at most `hexsz` characters of the `hex` string to
 * binary data, `size` must be a multiple of 2, this operation
 * is in-place and `hex` shall be modified, returned pointer
 * is the same as `hex`
 */
uint8_t *hex2bin(char *hex, size_t hexsz);

/**
 * convert at most `binsz` bytes of the `bin` binary data to
 * hex characters, making it null-terminated, and `bin` shall
 * be invalid after convert, returned value shall be freed
 */
char *bin2hex(uint8_t *bin, size_t binsz);

/**
 * check if at most `size` characters of the `hex` string are
 * valid hex characters
 */
bool is_hexstr(const char *hex, size_t size);

/**
 * check if at most `size` characters of the `salt` string are
 * valid hex characters
 */
bool is_saltstr(const char *salt, size_t size);

uint8_t *digest_message_sha256(const uint8_t *message, size_t message_length);

static inline void clean_digest(uint8_t *digest)
{
	OPENSSL_free(digest);
}

void report_openssl_error(void);

#endif /* RAWNUMOP_H */
