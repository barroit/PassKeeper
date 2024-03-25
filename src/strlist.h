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

#ifndef STRLIST_H
#define STRLIST_H

struct strlist_elem
{
	char *str;
	void *ext;
};

struct strlist
{
	struct strlist_elem *items;
	size_t size;
	size_t capacity;
	bool   dupstr;
};

#define STRLIST_INIT_NODUP { 0 }
#define STRLIST_INIT_DUP   { .dupstr = true }

struct strlist_elem *strlist_append(struct strlist *sl, const char *str);

struct strlist_elem *strlist_append_nodup(struct strlist *sl, char *str);

void string_list_clear(struct strlist *sl, bool free_ext);

size_t string_list_split(struct strlist *sl, const char *str, char delim, int maxsplit);

static inline bool in_string_array(const char *strarr[], const char *item)
{
	while (*strarr && strcmp(item, *strarr))
	{
		strarr++;
	}
	return *strarr != NULL;
}

#endif /* STRLIST_H */