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

#include "command.h"
#include "strlist.h"
#include "algorithm.h"
#include "strbuf.h"

int cmd_count  (int argc,  const char **argv, const char *prefix);
int cmd_create (int argc,  const char **argv, const char *prefix);
int cmd_delete (int argc,  const char **argv, const char *prefix);
int cmd_help   (int argc,  const char **argv, const char *prefix);
int cmd_init   (int argc,  const char **argv, const char *prefix);
int cmd_makekey(int argc,  const char **argv, const char *prefix);
int cmd_read   (int argc,  const char **argv, const char *prefix);
int cmd_update (int argc,  const char **argv, const char *prefix);
int cmd_version(int argc,  const char **argv, const char *prefix);

int cmd_reset  (int argc,  const char **argv, const char *prefix);

#define iterate_command_list(iter)\
	array_iterate_each_t(iter, command_list, (iter)->name)

const struct cmdinfo command_list[] = {
	{ "count",    cmd_count,  USE_CREDDB },
	{ "create",   cmd_create, USE_CREDDB | USE_RECFILE },
	{ "delete",   cmd_delete, USE_CREDDB },
	{ "help",     cmd_help },
	{ "init",     cmd_init },
	{ "makekey",  cmd_makekey },
	{ "read",     cmd_read, USE_CREDDB },
	/* { "show",     cmd_show, USE_CREDDB  }, */
	{ "update",   cmd_update, USE_CREDDB | USE_RECFILE },
	{ "version",  cmd_version },
	/* { "validate", cmd_validate, USE_CREDDB  }, */
#ifdef PK_DEBUG
	{ "reset",    cmd_reset, USE_CREDDB },
#endif
	{ NULL },
};

const struct cmdinfo *find_command(const char *cmd)
{
	const struct cmdinfo *iter;

	iterate_command_list(iter)
	{
		if (!strcmp(cmd, iter->name))
		{
			return iter;
		}
	}

	return NULL;
}

static int command_distance_compar(const void *o1, const void *o2)
{
	struct strlist_elem *e1, *e2;

	e1 = (struct strlist_elem *)o1;
	e2 = (struct strlist_elem *)o2;

	return e1->ext - e2->ext;
}

char **get_approximate_command(const char *cmd)
{
	const struct cmdinfo *iter;
	struct strlist sl = STRLIST_INIT_NODUP;
	size_t i, most_similar;

	iterate_command_list(iter)
	{
		strlist_push(&sl, iter->name);
	}

	array_for_each(i, sl.size)
	{
		if (starts_with(sl.elvec[i].str, cmd))
		{
			sl.elvec[i].ext = 0;
		}
		else
		{
			sl.elvec[i].ext = levenshtein_w(cmd, sl.elvec[i].str,
							 1, 3, 2, 0);
		}

	}

	MSORT(sl.elvec, sl.size, command_distance_compar);

	if ((most_similar = sl.elvec[0].ext) > 5)
	{
		return NULL;
	}

	for (i = 0; i < sl.size && sl.elvec[i].ext == 0; i++);

	if (i == sl.size)
	{
		if (sl.size > 7)
		{
			sl.size = 7;
		}
	}

	for (i = 0; i < sl.size && most_similar == sl.elvec[i].ext; i++);

	char **buf;

	buf = strlist_to_array_lim(&sl, i);
	strlist_destroy(&sl, false);

	return buf;
}
