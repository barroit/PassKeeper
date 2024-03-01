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