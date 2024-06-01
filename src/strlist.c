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

#include "strlist.h"

static void strlist_erase_at(struct strlist *sl, size_t idx, bool rmext)
{
	if (sl->dupstr)
	{
		free(sl->elvec[idx].str);
	}

	if (rmext)
	{
		free((void *)sl->elvec[idx].ext);
	}
}

void strlist_destroy(struct strlist *sl, bool rmext)
{
	for (; sl->size; sl->size--)
	{
		strlist_erase_at(sl, sl->size - 1, rmext);
	}

	free(sl->elvec);
	sl->capacity = 0;
}

void strlist_trunc(struct strlist *sl, bool rmext)
{
	for (; sl->size > 0; sl->size--)
	{
		strlist_erase_at(sl, sl->size - 1, rmext);
	}
}

static struct strlist_elem *strlist_push_nodup(struct strlist *sl, char *str)
{
	struct strlist_elem *el;

	CAPACITY_GROW(sl->elvec, sl->size + 1, sl->capacity);
	el = &sl->elvec[sl->size++];

	el->str = str;
	el->ext = 0;

	return el;
}

struct strlist_elem *strlist_push(struct strlist *sl, const char *str)
{
	return strlist_push_nodup(sl, sl->dupstr ? strdup(str) : (char *)str);
}

struct strlist_elem *strlist_pop(struct strlist *sl)
{
	if (sl->size == 0)
	{
		return NULL;
	}

	return sl->elvec + (sl->size-- - 1);
}

size_t strlist_split(struct strlist *sl, const char *str, char delim, int maxsplit)
{
	if (!sl->dupstr)
	{
		bug("strlist_split() shall be called with dupstr set to true");
	}

	size_t count;
	const char *delim_pos;

	count = 0;
	while (39)
	{
		count++;

		if (maxsplit >= 0 && count >= (unsigned)maxsplit)
		{
			strlist_push(sl, str);
			return count;
		}

		delim_pos = strchr(str, delim);

		if(delim_pos)
		{
			strlist_push_nodup(sl, strndup(str, delim_pos - str));
			str = delim_pos + 1;
		}
		else
		{
			strlist_push(sl, str);
			return count;
		}
	}
}

void strlist_filter(
	struct strlist *sl,
	bool (*compar)(struct strlist_elem *),
	bool free_ext)
{
	size_t i, ii, iii;
	size_t track[sl->size], tracksz;

	tracksz = 0;
	array_for_each(i, sl->size)
	{
		if (!compar(sl->elvec + i))
		{
			track[tracksz++] = i;
		}
	}

	iii = 0;
	array_for_each(i, tracksz)
	{
		strlist_erase_at(sl, track[i], free_ext);

		ii = track[i];
		iii++;

		while (i < tracksz - 1 ?
			++ii != track[i + 1] :
			 ++ii < sl->size)
		{
			sl->elvec[ii - iii] = sl->elvec[ii];
		}

		sl->size--;
	}
}

char *strlist_join(
	struct strlist *sl, const char *separator,
	enum strlist_join_ext_pos join_pos)
{
	char *buf0, *buf;
	size_t bufsz, bufcap, prevsz;
	size_t strsz, extsz, sepsz;

	bufsz = 0;
	bufcap = 0;
	sepsz = strlen(separator);

	buf0 = NULL;
	buf = buf0;

	struct strlist_elem *el;

	for (el = sl->elvec; el - sl->elvec < sl->size; el++)
	{
		prevsz = bufsz;
		strsz = strlen(el->str);
		extsz = join_pos != EXT_JOIN_NONE &&
			 el->ext != 0 ? strlen((const char *)el->ext) : 0;
		bufsz += strsz + extsz + sepsz;

		CAPACITY_GROW(buf0, bufsz, bufcap);
		buf = buf0;

		buf += prevsz;
		if (join_pos == EXT_JOIN_HEAD)
		{
			memcpy(buf, (void *)el->ext, extsz);
			buf += extsz;
		}
		memcpy(buf, el->str, strsz);
		buf += strsz;
		if (join_pos == EXT_JOIN_TAIL)
		{
			memcpy(buf, (void *)el->ext, extsz);
			buf += extsz;
		}
		memcpy(buf, separator, sepsz);
	}

	buf0[bufsz - sepsz] = 0; /* remove the last seperator */

	return buf0;
}

char **strlist_to_array_routine(struct strlist *sl, size_t n)
{
	char **buf;
	size_t i;

	if (n == 0 || n > sl->size)
	{
		n = sl->size;
	}

	buf = xmalloc(st_mult(n + 1, sizeof(char *)));

	array_for_each(i, n)
	{
		buf[i] = strdup(sl->elvec[i].str);
	}

	buf[i] = NULL;

	return buf;
}

void strarr_free(char **array)
{
	char **iter;

	array_iterate_each(iter, array)
	{
		free(*iter);
	}

	free(array);
}
