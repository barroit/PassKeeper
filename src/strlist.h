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
	struct strlist_elem *elvec;
	size_t size;
	size_t capacity;
	bool   dupstr;
};

enum strlist_join_ext_pos
{
	EXT_JOIN_NONE,
	EXT_JOIN_HEAD,
	EXT_JOIN_TAIL,
};

typedef bool (*strlist_filter_cb_t)(struct strlist_elem *);

#define STRLIST_INIT_NODUP  { 0 }
#define STRLIST_INIT_DUPSTR { .dupstr = true }
#define STRLIST_INIT_PTR_NODUP  &(struct strlist)STRLIST_INIT_NODUP
#define STRLIST_INIT_PTR_DUPSTR &(struct strlist){ .dupstr = true }

void strlist_destroy(struct strlist *sl, bool rmext);

void strlist_trunc(struct strlist *sl, bool rmext);

struct strlist_elem *strlist_push(struct strlist *sl, const char *str);

struct strlist_elem *strlist_pop(struct strlist *sl);

size_t strlist_split(struct strlist *sl, const char *str, char delim, int maxsplit);

void strlist_filter(struct strlist *sl, strlist_filter_cb_t pass, bool rmext);

char *strlist_join(struct strlist *sl, char *separator, enum strlist_join_ext_pos join_pos);

#define strlist_foreach(list, iter)\
	for (iter = list->elvec; iter - list->elvec < list->size; iter++)

#define list_have_next(pos)\
	( pos->next != NULL )

#define list_for_each(pos, head)\
	for (pos = (head)->next; list_have_next(pos); pos = pos->next)

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
void free_string_array(char **arr);

#define list_foreach(iter, head)\
	for (iter = head; iter->next != NULL; iter = iter->next)

#endif /* STRLIST_H */
