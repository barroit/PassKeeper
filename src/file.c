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

#include "file.h"

// char *read_content(const char *pathname, size_t *size)
// {
// 	FILE *file;
// 	if ((file = fopen(pathname, "r")) == NULL)
// 	{
// 		return NULL;
// 	}

// 	stringbuffer *buf;
// 	char c, *res;

// 	buf = sballoc(125);
// 	while ((c = fgetc(file)) != EOF)
// 	{
// 		sbputc(buf, c);
// 	}

// 	res = u8substr(buf->data, 0, 0);

// 	if (size != NULL)
// 	{
// 		*size = buf->size;
// 	}

// 	sbfree(buf);
// 	fclose(file);

// 	return res;
// }

// int prepare_folder(const char *pathname)
// {
// 	char *dirname;
// 	if ((dirname = prefix(pathname)) == NULL)
// 	{
// 		return PK_INVALID_PATHNAME;
// 	}

// 	int rc;

// 	rc = mkdir_p(dirname);
// 	free(dirname);

// 	return rc;
// }

// char *prefix(const char *pathname)
// {
// 	const char *seperator;
// 	if ((seperator = strrchr(pathname, '/')) == NULL && (seperator = strrchr(pathname, '\\')) == NULL)
// 	{
// 		return NULL;
// 	}

// 	char *dirname;
// 	ptrdiff_t dnlen; /* directory name length */

// 	dnlen = seperator - pathname;
// 	if ((dirname = malloc(dnlen + 1)) == NULL)
// 	{
// 		return NULL;
// 	}

// 	memcpy(dirname, pathname, dnlen);
// 	dirname[dnlen] = 0;

// 	return dirname;
// }

// #ifdef __linux__
// #include <sys/stat.h>
// #endif

// /* no error if existing */
// int mkdir_p(const char *pathname)
// {
// 	if (exists(pathname))
// 	{
// 		return PK_SUCCESS;
// 	}

// 	int rc;

// #ifdef __linux__
// 	rc = mkdir(pathname, 0755);
// #else
// 	rc = mkdir(pathname);
// #endif

// 	return rc == 0 ? PK_SUCCESS : PK_MKDIR_FAILURE;
// }