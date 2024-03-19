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

struct strlist_elem *strlist_append(struct strlist *sl, const char *str)
{
	return strlist_append_nodup(sl, sl->dupstr ? strdup(str) : (char *)str);
}

struct strlist_elem *strlist_append_nodup(struct strlist *sl, char *str)
{
	struct strlist_elem *el;

	CAPACITY_GROW(sl->items, sl->size + 1, sl->capacity);
	el = &sl->items[sl->size++];

	el->str = str;
	el->ext = NULL;

	return el;
}

void string_list_clear(struct strlist *sl, bool free_ext)
{
	bool prev_dupstr;

	for (; sl->size; sl->size--)
	{
		if (sl->dupstr)
		{
			free(sl->items[sl->size - 1].str);
		}

		if (free_ext)
		{
			free(sl->items[sl->size - 1].ext);
		}
	}

	free(sl->items);

	prev_dupstr = sl->dupstr;
	memset(sl, 0, sizeof(struct strlist));
	sl->dupstr = prev_dupstr;
}

size_t string_list_split(struct strlist *sl, const char *str, char delim, int maxsplit)
{
	assert(sl->dupstr);

	size_t count;
	const char *delim_pos;

	count = 0;

	while (1)
	{
		count++;

		if (maxsplit >= 0 && count >= (unsigned)maxsplit)
		{
			strlist_append(sl, str);
			return count;
		}

		delim_pos = strchr(str, delim);

		if(delim_pos)
		{
			strlist_append_nodup(sl, xmemdup_str(str, delim_pos - str));
			str = delim_pos + 1;
		}
		else
		{
			strlist_append(sl, str);
			return count;
		}
	}
}