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

void strlist_clear(struct strlist *sl, bool free_ext);

size_t strlist_split(struct strlist *sl, const char *str, char delim, int maxsplit);

/**
 * find `str` in `arr`, the last element of `arr` must be NULL
 */
static inline bool string_in_array(const char *str, const char *const *arr)
{
	while (*arr && strcmp(str, *arr))
	{
		arr++;
	}

	return *arr != NULL;
}

/**
 * convert a strlist to an array, duplicate each string, terminated
 * with a NULL
 */
char **strlist_to_array(struct strlist *sl);

/**
 * free a NULL terminated array
 */
void free_array(char **arr);

#endif /* STRLIST_H */