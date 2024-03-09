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

static struct command_info commands[] = {
	{ "count",	cmd_count },
	// { "create",	cmd_create },
	// { "delete",	cmd_delete },
	// { "help",	cmd_help },
	// { "init",	cmd_init },
	// { "read",	cmd_read },
	// { "update",	cmd_update },
	// { "version",	cmd_version },
	{ "dump",	NULL }, /* reserved */
	{ "source",	NULL }, /* reserved */
	{ NULL },
};

struct command_info *find_command(const char *cmdname)
{
	struct command_info *cmditer;

	cmditer = commands;
	while ((*cmditer).name)
	{
		if (!strcmp((*cmditer++).name, cmdname))
		{
			return cmditer - 1;
		}
	}

	return NULL;
}

void execute_command(struct command_info *command, int argc, const char **argv)
{
	if (command->handle == NULL)
	{
		bug("command '%s' does not have a handle", command->name);
	}

	command->handle(argc, argv);
}