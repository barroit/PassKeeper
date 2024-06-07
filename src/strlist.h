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
	intptr_t ext;
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

#define STRLIST_INIT_NODUP  { 0 }
#define STRLIST_INIT_DUPSTR { .dupstr = true }
#define STRLIST_INIT_PTR_NODUP  &(struct strlist)STRLIST_INIT_NODUP
#define STRLIST_INIT_PTR_DUPSTR &(struct strlist){ .dupstr = true }

void strlist_destroy(struct strlist *sl, bool rmext);

void strlist_trunc(struct strlist *sl, bool rmext);

struct strlist_elem *strlist_push(struct strlist *sl, const char *str);

struct strlist_elem *strlist_pop(struct strlist *sl);

size_t strlist_split(struct strlist *sl, const char *str, char delim, int maxsplit);

void strlist_filter(struct strlist *sl, bool (*compar)(struct strlist_elem *), bool rmext);

/**
 * join the string, the element in strlist is separated
 * by separator
 * 
 * this function only works on the ext is string when
 * join_pos is not EXT_JOIN_NONE
 */
char *strlist_join(const struct strlist *sl, const char *separator, enum strlist_join_ext_pos join_pos);

char **strlist_to_array_routine(struct strlist *sl, size_t n);

/**
 * convert a strlist to an array, duplicate each string, terminated
 * with a NULL
 */
#define strlist_to_array(sl) strlist_to_array_routine(sl, 0)

/**
 * same as strlist_to_array expect it convert almost n size
 * elements in elvec
 */
#define strlist_to_array_lim strlist_to_array_routine

/**
 * free a NULL terminated array
 */
void strarr_free(char **arr);

#endif /* STRLIST_H */
