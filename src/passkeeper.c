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