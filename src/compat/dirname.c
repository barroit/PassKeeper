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
		if (*iter == DIRSEPCHAR)
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
		return DIRSEPSTR;
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
				return DIRSEPSTR;
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
