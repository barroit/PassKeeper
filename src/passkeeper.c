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

#include "parse-option.h"
#include "strbuf.h"
#include "filesys.h"

int cmd_count(int argc,   const char **argv, const char *prefix);
int cmd_create(int argc,  const char **argv, const char *prefix);
int cmd_delete(int argc,  const char **argv, const char *prefix);
int cmd_help(int argc,    const char **argv, const char *prefix);
int cmd_init(int argc,    const char **argv, const char *prefix);
int cmd_makekey(int argc, const char **argv, const char *prefix);
int cmd_read(int argc,    const char **argv, const char *prefix);
int cmd_update(int argc,  const char **argv, const char *prefix);
int cmd_version(int argc, const char **argv, const char *prefix);

enum precheck_flag
{
	/* credfile shall exist */
	USE_CREDFILE = 1 << 0,
	/**
	 * recfile shall be available for open/create, this file
	 * is not created by this flag, and the file shall be
	 * removed by command handler after command executing
	 */
	USE_RECFILE  = 1 << 1,
};

struct command_info
{
	const char *name;
	int (*handle)(int argc, const char **argv, const char *prefix);
	enum precheck_flag prechecks;
};

static struct command_info commands[] = {
	{ "count",    cmd_count,  USE_CREDFILE },
	{ "create",   cmd_create, USE_CREDFILE | USE_RECFILE },
	{ "delete",   cmd_delete, USE_CREDFILE },
	{ "help",     cmd_help },
	{ "init",     cmd_init },
	{ "makekey",  cmd_makekey },
	{ "read",     cmd_read, USE_CREDFILE },
	/* { "show",     cmd_show, USE_CREDFILE  }, */
	{ "update",   cmd_update, USE_CREDFILE | USE_RECFILE },
	{ "version",  cmd_version },
	/* { "validate", cmd_validate, USE_CREDFILE  }, */
	{ NULL },
};

static struct
{
	const char *db_path;
	const char *key_path;
	const char *recfile;
	const char *editor;
	const char *spinner_style;
} environment = {
	.spinner_style = "0",
};

const char *const cmd_pk_usages[] = {
	"pk [--db-path <file>] [--key-path <file>] [--editor <name>] \n"
	"   [--[no]-spinner[=<style>]] <command> [<args>]",
	NULL,
};

const struct option cmd_pk_options[] = {
	OPTION_HIDDEN_PATHNAME(0, "db-path", &environment.db_path),
	OPTION_HIDDEN_PATHNAME(0, "key-path", &environment.key_path),
	OPTION_HIDDEN_STRING(0, "editor", &environment.editor),
	OPTION_HIDDEN_OPTARG_ALLONEG(0, "spinner", &environment.spinner_style,
					"default"),
	OPTION_COMMAND("count",   "Count the number of records"),
	OPTION_COMMAND("create",  "Create a record"),
	OPTION_COMMAND("delete",  "Delete a record"),
	OPTION_COMMAND("help",    "Display help information about PassKeeper"),
	OPTION_COMMAND("init",    "Initialize database files for "
				  "storing credentials"),
	OPTION_COMMAND("makekey", "Generate random bytes using a CSPRNG"),
	OPTION_COMMAND("read",    "Read a record"),
	OPTION_COMMAND("update",  "Update a record"),
	OPTION_COMMAND("version", "Display version information about "
				  "PassKeeper"),
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
	size_t size;
	char *buf;

	size = 96;
	while (39)
	{
		buf = xmalloc(size);

		if (getcwd(buf, size) == buf)
		{
			return buf;
		}

		if (errno == ERANGE)
		{
			free(buf);
			size = fixed_growth(size);
			continue;
		}

		die_errno("Unable to get the current working directory");
	}

	return buf;
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

static inline FORCEINLINE void adaptive_setenv(
	const char *name, const char *value, const char *defval)
{
	setenv(name, value == NULL ? defval : value, value != NULL);
}

static int handle_global_options(
	int argc, const char **argv, const char *prefix)
{
	option_usage_alignment = 13;
	argc = parse_options(argc, argv, prefix, cmd_pk_options, cmd_pk_usages,
			      PARSER_STOP_AT_NON_OPTION | PARSER_NO_SHORT_HELP);
	option_usage_alignment = OPTION_USAGE_ALIGNMENT;

	adaptive_setenv(PK_CRED_DB, environment.db_path,  PK_CRED_DB_NM);
	adaptive_setenv(PK_CRED_KY, environment.key_path, PK_CRED_KY_NM);
	adaptive_setenv(PK_RECFILE, environment.recfile,  PK_RECFILE_NM);

	if (environment.editor)
	{
		setenv(PK_EDITOR, environment.editor, 1);
	}

	if (environment.spinner_style)
	{
		setenv(PK_SPINNER, environment.spinner_style, 1);
	}
	else
	{
		unsetenv(PK_SPINNER);
	}

	return argc;
}

static bool is_skip_precheck(int argc, const char **argv)
{
	for ( ; argc > 0; argc--)
	{
		if (!strcmp(argv[argc - 1], "-h"))
		{
			return true;
		}
	}

	return false;
}

static void precheck_command(enum precheck_flag prechecks)
{
	if (prechecks & USE_RECFILE)
	{
		prepare_file_directory(force_getenv(PK_RECFILE));
	}

	if (prechecks & USE_CREDFILE)
	{
		struct stat st;
		const char *credfl = force_getenv(PK_CRED_DB);

		if (stat(credfl, &st) != 0)
		{
			die_errno("Couldn't access file '%s'", credfl);
		}
		else if (!S_ISREG(st.st_mode))
		{
			die("File '%s' is not a regular file", credfl);
		}
		else if (test_file_permission(credfl, &st, R_OK | W_OK) != 0)
		{
			die("Access denied by file '%s'", credfl);
		}
	}
}

int main(int argc, const char **argv)
{
	const char *prefix;
	struct command_info *command;

	argv++;
	argc--;

	prefix = resolve_working_dir();

	argc = handle_global_options(argc, argv, prefix);

	calibrate_argv(&argc, &argv);

	command = find_command(*argv);
	if (command == NULL)
	{
		bug("your calibrate_argv() is broken");
	}

	argc--;
	argv++;

	if (!is_skip_precheck(argc, argv))
	{
		precheck_command(command->prechecks);
	}

	exit(command->handle(argc, argv, prefix) & 0xff);
}
