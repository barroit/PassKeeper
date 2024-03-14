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

int cmd_count(int argc, const char **argv, const char *prefix);
int cmd_create(int argc, const char **argv, const char *prefix);
int cmd_delete(int argc, const char **argv, const char *prefix);
int cmd_help(int argc, const char **argv, const char *prefix);
int cmd_init(int argc, const char **argv, const char *prefix);
int cmd_read(int argc, const char **argv, const char *prefix);
int cmd_update(int argc, const char **argv, const char *prefix);
int cmd_version(int argc, const char **argv, const char *prefix);

const char pk_usages[] = "pk <command> [<args>]";

struct command_info
{
	const char *name;
	int (*handle)(int argc, const char **argv, const char *prefix);
	const char *help;
};

static struct command_info commands[] = {
	{ "count",	cmd_count,	"Count the number of records" },
	// { "create",	cmd_create,	"" },
	// { "delete",	cmd_delete,	"" },
	{ "help",	cmd_help,	"Display help information about PassKeeper" },
	{ "init",	cmd_init,	"Initialize database files for storing credentials" },
	// { "read",	cmd_read,	"" },
	// { "update",	cmd_update,	"" },
	{ "version",	cmd_version,	"Display version information about PassKeeper" },
	// { "dump",	NULL,		"" }, /* reserved */
	// { "source",	NULL,		"" }, /* reserved */
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

static void execute_command(struct command_info *command, const char *prefix, int argc, const char **argv)
{
	if (command->handle == NULL)
	{
		bug("command '%s' has no handler", command->name);
	}

	exit(command->handle(argc, argv, prefix));
}

static char *resolve_executable_prefix(const char *execpath)
{
	static char *ret;
	const char *rtsep;
	ptrdiff_t sz;

	/**
	 * Linux:
	 * pk
	 * ./pk
	 * ./build/debug/pk
	 * /home/barroit/Workspace/PassKeeper/.local/bin/pk
	 *
	 * Windows:
	 * pk.exe
	 * .\Desktop\pk.exe
	 * C:\Users\Administrator\Desktop\pk.exe
	 */

	rtsep = strrchr(execpath, '/');

	if (rtsep == NULL)
	{
		rtsep = strrchr(execpath, '\\');

		if (rtsep == NULL)
		{
			execpath = get_user_home();
			rtsep = execpath + strlen(execpath);
		}
	}

	ret = replace_char(execpath, '\\', '/');

	sz = (rtsep + 1) - execpath;
	ret = xrealloc(ret, sz + 1);

	ret[sz] = 0;
	return ret;
}

/* do not use '__' as parameter prefix, program name should NOT be in the argv */
static void calibrate_argv(int *argc0, const char ***argv0)
{
	int argc;
	const char **argv;

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

static void list_command_help(void)
{
	const struct command_info *iter;

	iter = commands;
	while (iter->name)
	{
		size_t pos;

		pos = printf("   %s", iter->name);
		print_help(iter->help, pos, stdout);
		putchar('\n');

		iter++;
	}
}

extern int option_usage_width;

void show_pk_help(void)
{
	printf_ln("usage: %s", pk_usages);
	putchar('\n');
	puts("These are common PassKeeper commands used in various situations:");

	option_usage_width = 13;
	list_command_help();
}

void show_unknow_usage(const char *name)
{
	fprintf_ln(stderr, "pk: '%s' is not a passkeeper command. See 'pk help'", name);
}

int main(int argc, const char **argv)
{
	char *prefix;
	struct command_info *command;
	const char *cmdname;

	prefix = resolve_executable_prefix(*argv);

	argc--;
	argv++;

	calibrate_argv(&argc, &argv);

	cmdname = argv[0];
	if ((command = find_command(cmdname)) == NULL)
	{
		show_unknow_usage(cmdname);
		return EXIT_FAILURE;
	}

	execute_command(command, prefix, --argc, ++argv);

	return EXIT_SUCCESS;
}