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

/**
 * The strdup() function shall return a pointer to a new string, which is a
 * duplicate of the string pointed to by s. The returned pointer can be passed
 * to free(). A null pointer is returned if the new string cannot be created.
 * 
 * The strndup() function shall be equivalent to the strdup() function,
 * duplicating the provided s in a new block of memory allocated as if by using
 * malloc(), with the exception being that strndup() copies at most size plus
 * one bytes into the newly allocated memory, terminating the new string with a
 * NUL character. If the length of s is larger than size, only size bytes shall
 * be duplicated. If size is larger than the length of s, all bytes in s shall
 * be copied into the new memory buffer, including the terminating NUL character.
 * The newly created string shall always be properly terminated.
 * 
 * 
 * The strdup() function shall return a pointer to a new string on success.
 * Otherwise, it shall return a null pointer and set errno to indicate the error.
 * 
 * Upon successful completion, the strndup() function shall return a pointer to
 * the newly allocated memory containing the duplicated string. Otherwise, it
 * shall return a null pointer and set errno to indicate the error.
 * 
 * 
 * These functions shall fail if:
 * [ENOMEM]
 *     Storage space available is insufficient.
 */
char *pk_strdup(const char *s)
{
	char *buf;
	size_t bufsz;

	bufsz = strlen(s) + 1;
	if ((buf = malloc(bufsz)) == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	memcpy(buf, s, bufsz);

	return buf;
}

char *pk_strndup(const char *s, size_t size)
{
	char *buf;
	size_t strsz;

	if ((strsz = strlen(s)) < size)
	{
		size = strsz;
	}

	if ((buf = malloc(size + 1)) == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	memcpy(buf, s, size);
	buf[size] = 0;

	return buf;
}
