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
	{ "count",	cmd_count_usages,	cmd_count_options },
	{ "create",	cmd_create_usages,	cmd_create_options },
	{ "delete",	cmd_delete_usages,	cmd_delete_options },
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

void list_passkeeper_commands();

void handle_command_not_found(const char *name)
{
	fprintf_ln(stderr, "pk: '%s' is not a passkeeper command. See 'pk help pk'", name);
	exit(129);
}

void handle_main_command_help(void)
{
	printf_ln("usage: %s", "pk <command> [<args>]");
	putchar('\n');
	puts("These are common PassKeeper commands used in various situations:");

	list_passkeeper_commands();
	putchar('\n'); /* acts like other helpers */
	exit(129);
}

int cmd_help(UNUSED int argc, const char **argv, UNUSED const char *prefix)
{
	const char *cmdname;
	const struct command_helper *helper;
	bool is_main_command;

	cmdname = *argv;
	is_main_command = cmdname && !strcmp(cmdname, "pk");

	if (!is_main_command && !is_command(cmdname))
	{
		handle_command_not_found(cmdname);
	}
	else if (is_main_command)
	{
		handle_main_command_help();
	}

	helper = find_command_helper(cmdname);
	if (helper == NULL)
	{
		bug("you definitely forgot to add %s to helpers", cmdname);
	}

	parse_options(1, (const char *[]){ "-h" }, prefix, helper->options, helper->usages, 0);

	return 0;
}