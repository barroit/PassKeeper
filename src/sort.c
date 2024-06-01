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

#include "algorithm.h"

static void merge_sort_next(
	void *array, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *),
	uint8_t *buf0)
{
	if (nmemb <= 1)
	{
		return;
	}

	uint8_t *buf;
	size_t nl, nr;
	void *al, *ar;

	nl = nmemb / 2;
	nr = nmemb - nl;

	al = array;
	ar = (uint8_t *)array + (nl * size);

	merge_sort_next(al, nl, size, compar, buf0);
	merge_sort_next(ar, nr, size, compar, buf0);

	buf = buf0;

	while (nl > 0 && nr > 0)
	{
		if (compar(al, ar) > 0)
		{
			memcpy(buf, ar, size);
			buf += size;
			ar += size;
			nr--;
		}
		else
		{
			memcpy(buf, al, size);
			buf += size;
			al += size;
			nl--;
		}
	}

	if (nl > 0)
	{
		memcpy(buf, al, nl * size);
	}

	memcpy(array, buf0, (nmemb - nr) * size);
}

void merge_sort(
	void *array, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *))
{
	uint8_t *buf;

	buf = xmalloc(st_mult(nmemb, size));
	merge_sort_next(array, nmemb, size, compar, buf);

	free(buf);
}

