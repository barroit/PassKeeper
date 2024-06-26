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

ssize_t pk_getline(
	char **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	if (lineptr == NULL || n == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	else if (ferror(stream)) /* no null-check */
	{
		errno = EINVAL;
		return -1;
	}

	char c, *ptr;
	size_t len;

	if ((c = fgetc(stream)) == EOF)
	{
		return -1;
	}

	if (*lineptr == NULL)
	{
		*n = fixed_growth(64);

		if ((*lineptr = malloc(*n)) == NULL)
		{
			errno = ENOMEM;
			return -1;
		}
	}

	ptr = *lineptr;
	len = 0;

	while (c != EOF)
	{
		if (ptr - *lineptr >= *n - 1) /* -1 for nulterm */
		{
			*n = fixed_growth(*n);

			ptr = realloc(ptr, *n);
		}

		*ptr++ = c;
		len++;

		if (c == '\n')
		{
			break;
		}

		c = fgetc(stream);
	}

	*ptr = 0;

	return len;
}
