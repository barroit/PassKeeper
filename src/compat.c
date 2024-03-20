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

char *pk_strchrnul(const char *s, int c)
{
	while (*s && *s != c)
	{
		s++;
	}

	return (char *)s;
}

int pk_setenv(const char *name, const char *value, int replace)
{
	if (name == NULL || value == NULL || strchr(name, '='))
	{
		return -1;
	}

	if (getenv(name) && !replace)
	{
		return 0;
	}

	size_t ll, rl;
	char *buf;

	ll = strlen(name);
	rl = strlen(value);
	buf = xmalloc(ll + rl + 2);

	memcpy(buf, name, ll);
	buf[ll++] = '=';
	memcpy(buf + ll, value, rl);
	buf[ll + rl] = 0;

	return putenv(buf);
}

/**
 * The dirname() function shall take a pointer to a character string that
 * contains a pathname, and return a pointer to a string that is a
 * pathname of the parent directory of that file. The dirname() function
 * shall not perform pathname resolution; the result shall not be
 * affected by whether or not path exists or by its file type. Trailing
 * '/' characters in the path that are not also leading '/' characters
 * shall not be counted as part of the path.
 *
 * If path does not contain a '/', then dirname() shall return a pointer
 * to the string ".". If path is a null pointer or points to an empty
 * string, dirname() shall return a pointer to the string "." .
 *
 * The dirname() function may modify the string pointed to by path, and
 * may return a pointer to static storage that may then be overwritten by
 * a subsequent call to dirname().
 */
char *pk_dirname(char *path)
{
	if (path == NULL || *path == 0 || *path == '.' || !strcmp(path, ".."))
	{
		return ".";
	}

	char *next_sep, *prev_sep, *iter;
	ptrdiff_t plen;

	next_sep = NULL;
	prev_sep = NULL;
	iter = path;

	while (*iter)
	{
		if (*iter == '/')
		{
			prev_sep = next_sep;
			next_sep = iter;
		}

		iter++;
	}

	/* not found */
	if (next_sep == NULL)
	{
		return ".";
	}
	/* path is '/' */
	else if (next_sep == path)
	{
		return "/";
	}
	else
	{
		/* trailing '/' */
		if (*(next_sep + 1) == 0)
		{
			/* path like 'usr/' */
			if (prev_sep == NULL)
			{
				return ".";
			}

			/* path like '/usr/' */
			if (prev_sep == path)
			{
				return "/";
			}

			plen = prev_sep - path; 
		}
		/* path like '/usr/lib' or 'usr/lib' */
		else
		{
			plen = next_sep - path;
		}

		path = xrealloc(path, plen);
		path[plen] = 0;
		return path;
	}
}