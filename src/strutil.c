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

#include "strutil.h"

char *mkspase(size_t length)
{
	char *dest;
	if ((dest = malloc(length + 1)) == NULL)
	{
		return NULL;
	}

	memset(dest, ' ', length); 
	dest[length] = 0;

	return dest;
}

char *concat(const char *str1, const char *str2)
{
	if (str1 == NULL || str2 == NULL)
	{
		return NULL;
	}

	size_t destlen, len1, len2;
	char *dest;

	len1 = strlen(str1);
	len2 = strlen(str2);
	destlen = len1 + len2;

	if ((dest = malloc(destlen + 1)) == NULL)
	{
		return NULL;
	}

	memcpy(dest, str1, len1);
	memcpy(dest + len1, str2, len2);
	dest[destlen] = 0;

	return dest;
}

char *substr(const char *src, size_t start, size_t length)
{
	if (src == NULL)
	{
		return NULL;
	}

	if (length == 0)
	{
		length = strlen(src) - start;
	}

	char *dest;
	if ((dest = malloc(length + 1)) == NULL)
	{
	    return NULL;
	}

	memcpy(dest, src + start, length);
	dest[length] = 0;

	return dest;
}

bool is_empty_string(const char *str)
{
	return str == NULL ? true : *str == '\0';
}

#define U8CNAR_MASK	0xC0
#define U8CNAR_MARKER	0x80

bool is_start_byte(const char c)
{
	return (c & U8CNAR_MASK) != U8CNAR_MARKER;
}

size_t u8strlen(const char *iter)
{
	size_t length;

	length = 0;
	while (*iter)
	{
		if (is_start_byte(*iter++))
		{
			length++;
		}
	}

	return length;
}

char *u8substr(const char *iter, size_t start_index, size_t substr_lenght)
{
	if (iter == NULL)
	{
		return NULL;
	}

	const char *head;
	size_t char_count;
	bool unknow_length, finish_calc;

	head = NULL;
	char_count = 0;
	unknow_length = substr_lenght == 0;
	finish_calc = false;
	while (*iter)
	{
		if (!is_start_byte(*iter))
		{
			iter++;
			continue;
		}

		if (finish_calc)
		{
			break;
		}

		iter++;
		char_count++;

		if (char_count - 1 < start_index) /* char_count - 1 for comparing index */
		{
			continue;
		}

		if (char_count - 1 == start_index) /* also index */
		{
			head = iter - 1;
		}

		if (unknow_length)
		{
			continue; /* let iter goes to the end */
		}

		substr_lenght--;

		if (substr_lenght == 0)
		{
			finish_calc = true;
		}
	}

	if (head == NULL) /* start_index out of size */
	{
		return NULL;
	}
 
	char *dest;
	ptrdiff_t destsz;

	destsz = iter - head;
	if ((dest = malloc(destsz + 1)) == NULL)
	{
	    return NULL;
	}

	memcpy(dest, head, destsz);
	dest[destsz] = 0;

	return dest;
}