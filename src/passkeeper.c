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

#include "environment.h"
#include "command.h"

static void preprocess_argv(int argc, const char **argv)
{
	/* like git, turn "pk <command> --help" into "pk help <command>" */
	if (argc > 1 && !strcmp("--help", argv[1]))
	{
		argv[1] = argv[0];
		argv[0] = "help";
	}
}

int main(int argc, const char **argv)
{
	initialize_environment();

	argc--;
	argv++;

	if (argc == 0)
	{
		const char *default_argv[] = { "help" };

		argc = 1;
		argv = default_argv;
	}

	struct command_info *command;

	preprocess_argv(argc, argv);

	if ((command = find_command(argv[0])) == NULL)
	{
		// no command founded
		return EXIT_FAILURE;
	}

	execute_command(command, --argc, ++argv);

	return EXIT_SUCCESS;
}