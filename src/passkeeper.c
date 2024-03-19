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
#include "filesys.h"

int cmd_count(int argc, const char **argv, const char *prefix);
int cmd_create(int argc, const char **argv, const char *prefix);
int cmd_delete(int argc, const char **argv, const char *prefix);
int cmd_help(int argc, const char **argv, const char *prefix);
int cmd_init(int argc, const char **argv, const char *prefix);
int cmd_read(int argc, const char **argv, const char *prefix);
int cmd_update(int argc, const char **argv, const char *prefix);
int cmd_version(int argc, const char **argv, const char *prefix);

struct command_info
{
	const char *name;
	int (*handle)(int argc, const char **argv, const char *prefix);
};

static struct command_info commands[] = {
	{ "count",   cmd_count },
	{ "create",  cmd_create },
	{ "delete",  cmd_delete },
	{ "help",    cmd_help },
	{ "init",    cmd_init },
	{ "read",    cmd_read },
	{ "update",  cmd_update },
	{ "version", cmd_version },
	{ NULL },
};

static const char *db_path;
static const char *key_path;

const char *const cmd_pk_usages[] = {
	"pk [--db-path <file>] [--key-path <file>] <command> [<args>]",
	NULL,
};

const struct option cmd_pk_options[] = {
	OPTION_HIDDEN_PATHNAME(0, "db-path", &db_path),
	OPTION_HIDDEN_PATHNAME(0, "key-path", &key_path),
	OPTION_COMMAND("count",   "Count the number of records"),
	OPTION_COMMAND("create",  "Create a record"),
	OPTION_COMMAND("delete",  "Delete a record"),
	OPTION_COMMAND("help",    "Display help information about PassKeeper"),
	OPTION_COMMAND("init",    "Initialize database files for storing credentials"),
	OPTION_COMMAND("read",    "Read a record"),
	OPTION_COMMAND("update",  "Update a record"),
	OPTION_COMMAND("version", "Display version information about PassKeeper"),
	OPTION_END(),
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

static char *resolve_working_dir(void)
{
	size_t size = 96;
	char *buf;

	while (39)
	{
		buf = xmalloc(size);

		if (getcwd(buf, size) == buf)
		{
			return buf;
		}

		if (errno != ERANGE)
		{
			die("permission denied while getting working directory");
		}

		free(buf);
		size = fixed_growth(size);
	}

	return buf;
}

static void execute_command(struct command_info *command, int argc, const char **argv, const char *prefix)
{
	if (command->handle == NULL)
	{
		bug("command '%s' has no handler", command->name);
	}

	exit(command->handle(argc, argv, prefix));
}

static const char *fallback_command[] = { "help", "pk", NULL };

void handle_command_not_found(const char *name);

/**
 * do not use '__' as parameter prefix
 * program name should NOT be in the argv
 */
static void calibrate_argv(int *argc, const char ***argv)
{
	/* turn "pk cmd --help" into "pk help cmd" */
	if (*argc > 1 && !strcmp((*argv)[1], "--help"))
	{
		(*argv)[1] = (*argv)[0];
		(*argv)[0] = "help";
	}
	/**
	 * - no command passed, set argv to "pk help pk"
	 * - turn "pk help" into "pk help pk"
	 */
	else if (*argc == 0 || (*argc == 1 && !strcmp(**argv, "help")))
	{
		*argc = 2;
		*argv = fallback_command;
	}
	else if (!strcmp(**argv, "pk"))
	{
		handle_command_not_found(**argv);
	}
	else if (!is_command(**argv))
	{
		/**
		 * turn "pk dummycmd" into "pk help dummycmd" so that
		 * command 'help' can generate messages
		 */
		fallback_command[1] = **argv;
		*argc = 2;
		*argv = fallback_command;
	}
}

static int handle_options(int argc, const char **argv, const char *prefix)
{
	option_usage_alignment = 13;
	argc = parse_options(argc, argv, prefix, cmd_pk_options, cmd_pk_usages, PARSER_STOP_AT_NON_OPTION | PARSER_NO_SHORT_HELP);
	option_usage_alignment = OPTION_USAGE_ALIGNMENT;

	if (db_path)
	{
		setenv(PK_CRED_DB, db_path, 1);
	}

	if (key_path)
	{
		setenv(PK_CRED_KY, key_path, 1);
	}

	return argc;
}

int main(int argc, const char **argv)
{
	const char *prefix;
	struct command_info *command;

	argv++;
	argc--;

	prefix = resolve_working_dir();

	argc = handle_options(argc, argv, prefix);

	calibrate_argv(&argc, &argv);

	command = find_command(*argv);
	if (command == NULL)
	{
		bug("your calibrate_argv() broken");
	}

	argc--;
	argv++;

	execute_command(command, argc, argv, prefix);

	return EXIT_SUCCESS;
}