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

#include "parse-options.h"

const char *const cmd_help_usages[] = {
	"pk help [<command>]",
	NULL,
};

const struct option cmd_help_options[] = {
	OPTION_END(),
};

struct command_helper
{
	const char *name;
	const char *const *usages;
	const struct option *options;
};

static const struct command_helper cmdhelps[] = {
	{ "count",	cmd_count_usages,	cmd_count_options },
	// { "create",	cmd__usages,	cmd__options },
	// { "delete",	cmd__usages,	cmd__options },
	{ "help",	cmd_help_usages,	cmd_help_options },
	// { "init",	cmd__usages,	cmd__options },
	// { "read",	cmd__usages,	cmd__options },
	// { "update",	cmd__usages,	cmd__options },
	{ "version",	cmd_version_usages,	cmd_version_options },
	// { "dump",	cmd__usages,	cmd__options },
	// { "source",	cmd__usages,	cmd__options },
	{ NULL },
};

static const struct command_helper *find_command_helper(const char *name)
{
	const struct command_helper *iter;

	iter = cmdhelps;
	while (iter->name)
	{
		if (!strcmp(name, iter->name))
		{
			return iter;
		}

		iter++;
	}

	return NULL;
}

void show_pk_help(void);
void show_unknow_usage(const char *name);
bool is_command(const char *cmdname);

int cmd_help(int argc, const char **argv, UNUSED const char *prefix)
{
	if (argc == 0)
	{
		show_pk_help();
		exit(EXIT_SUCCESS);
	}

	const char *cmdname;
	const struct command_helper *helper;

	cmdname = *argv;
	if (!is_command(cmdname))
	{
		show_unknow_usage(cmdname);
		exit(EXIT_FAILURE);
	}

	helper = find_command_helper(cmdname);
	if (helper == NULL)
	{
		bug("you definitely forgot to add %s to cmdhelps", cmdname);
	}

	parse_options(1, (const char *[]){ "-h" }, helper->options, helper->usages, 0);

	return 0;
}