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
