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

struct command_info
{
	const char *name;
	int (*handle)(int argc, const char **argv);
};

static struct command_info commands[] = {
	{ "count",	cmd_count },
	// { "create",	cmd_create },
	// { "delete",	cmd_delete },
	// { "help",	cmd_help },
	// { "init",	cmd_init },
	// { "read",	cmd_read },
	// { "update",	cmd_update },
	{ "version",	cmd_version },
	{ "dump",	NULL }, /* reserved */
	{ "source",	NULL }, /* reserved */
	{ NULL },
};

static struct command_info *find_command(const char *cmdname)
{
	struct command_info *cmditer;

	cmditer = commands;
	while (cmditer->name != NULL)
	{
		if (!strcmp(cmdname, cmditer++->name))
		{
			return cmditer - 1;
		}
	}

	return NULL;
}

bool is_command(const char *cmdname)
{
	return find_command(cmdname);
}

static void execute_command(struct command_info *command, int argc, const char **argv)
{
	if (command->handle == NULL)
	{
		bug("command '%s' has no handler", command->name);
	}

	command->handle(argc, argv);
}

/* do not use '__' as parameter prefix */
static void calibrate_argv(int *argc0, const char ***argv0)
{
	int argc;
	const char **argv;

	(*argc0)--;
	(*argv0)++;

	argc = *argc0;
	argv = *argv0;

	/* turn "pk cmd --help" into "pk help cmd" */
	if (argc > 1 && !strcmp(argv[1], "--help"))
	{
		argv[1] = argv[0];
		argv[0] = "help";
	}
	else if (argc == 0)
	{
		*argc0 = 1;
		**argv0 = "help";
	}
}

int main(int argc, const char **argv)
{
	struct command_info *command;

	calibrate_argv(&argc, &argv);

	if ((command = find_command(argv[0])) == NULL)
	{
		// no command founded
		return EXIT_FAILURE;
	}

	execute_command(command, --argc, ++argv);

	return EXIT_SUCCESS;
}