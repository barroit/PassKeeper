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

#include "parseopt.h"
#include "strbuf.h"

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

static const struct command_helper helpers[] = {
	{ "count",   cmd_count_usages,   cmd_count_options },
	{ "create",  cmd_create_usages,  cmd_create_options },
	{ "delete",  cmd_delete_usages,  cmd_delete_options },
	{ "help",    cmd_help_usages,    cmd_help_options },
	{ "init",    cmd_init_usages,    cmd_init_options },
	{ "makekey", cmd_makekey_usages, cmd_makekey_options },
	{ "read",    cmd_read_usages,    cmd_read_options },
	{ "update",  cmd_update_usages,  cmd_update_options },
	{ "version", cmd_version_usages, cmd_version_options },
	{ "pk",      cmd_pk_usages,      cmd_pk_options },
	{ NULL },
};

static const struct command_helper *find_command_helper(const char *name)
{
	const struct command_helper *iter;

	iter = helpers;
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

bool is_command(const char *cmdname);

void handle_command_not_found(const char *name)
{
	fprintfln(stderr, "pk: '%s' is not a passkeeper command. See 'pk help pk'", name);
	exit(129);
}

int cmd_help(UNUSED int argc, const char **argv, const char *prefix)
{
	const char *cmdname;
	const struct command_helper *helper;

	cmdname = *argv;

	if (!strcmp(cmdname, "-h"))
	{
		cmdname = "help";
	}

	if (!is_command(cmdname) && strcmp(cmdname, "pk"))
	{
		handle_command_not_found(cmdname);
	}

	helper = find_command_helper(cmdname);
	if (helper == NULL)
	{
		bug("you definitely forgot to add %s to helpers", cmdname);
	}

	if (!strcmp(cmdname, "pk"))
	{
		option_usage_alignment = 13;
	}

	parse_options(1, (const char *[]){ "-h" }, prefix, helper->options, helper->usages, 0);

	return 0;
}
