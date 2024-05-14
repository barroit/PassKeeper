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
#include "atexit-chain.h"

int cmd_count  (int argc,  const char **argv, const char *prefix);
int cmd_create (int argc,  const char **argv, const char *prefix);
int cmd_delete (int argc,  const char **argv, const char *prefix);
int cmd_help   (int argc,  const char **argv, const char *prefix);
int cmd_init   (int argc,  const char **argv, const char *prefix);
int cmd_makekey(int argc,  const char **argv, const char *prefix);
int cmd_read   (int argc,  const char **argv, const char *prefix);
int cmd_reset  (int argc,  const char **argv, const char *prefix);
int cmd_update (int argc,  const char **argv, const char *prefix);
int cmd_version(int argc,  const char **argv, const char *prefix);

#define USE_CREDFILE (1 << 0)
#define USE_RECFILE  (1 << 1)

struct command_info
{
	const char *name;
	int (*handle)(int argc, const char **argv, const char *prefix);
	unsigned precheck_flags;
};

static struct command_info commands[] = {
	{ "count",    cmd_count,  USE_CREDFILE },
	{ "create",   cmd_create, USE_CREDFILE | USE_RECFILE },
	{ "delete",   cmd_delete, USE_CREDFILE },
	{ "help",     cmd_help },
	{ "init",     cmd_init },
	{ "makekey",  cmd_makekey },
	{ "read",     cmd_read, USE_CREDFILE },
#ifdef PK_DEBUG
	{ "reset",    cmd_reset, USE_CREDFILE },
#endif
	/* { "show",     cmd_show, USE_CREDFILE  }, */
	{ "update",   cmd_update, USE_CREDFILE | USE_RECFILE },
	{ "version",  cmd_version },
	/* { "validate", cmd_validate, USE_CREDFILE  }, */
	{ NULL },
};

static const char *const cmd_pk_usages[] = {
	"pk [--cred-db <file>] [--cred-cc <file>] [--temp-rc <file>]\n"
	"   [--editor <name>] [--[no]-spinner[=<style>]] <command>\n"
	"   [<args>]",
	NULL,
};

static const struct option cmd_pk_options[] = {
	OPTION_FILENAME_H(0, "cred-db",  &cred_db_path),
	OPTION_FILENAME_H(0, "cred-cc",  &cred_cc_path),
	OPTION_FILENAME_H(0, "tmp-rec",  &tmp_rec_path),
	OPTION_STRING_H  (0, "editor",   &ext_editor),
	OPTION_OPTARG_HN (0, "spinner",  &spinner_style, "default"),

	OPTION_GROUP("database manipulation"),
	OPTION_COMMAND("init",    "Initialize database files for "
				  "storing credentials"),
	OPTION_COMMAND("create",  "Create a record"),
	OPTION_COMMAND("read",    "Read a record"),
	OPTION_COMMAND("update",  "Update a record"),
	OPTION_COMMAND("delete",  "Delete a record"),
	OPTION_COMMAND("count",   "Count the number of records"),

	OPTION_GROUP("utility"),
	OPTION_COMMAND("makekey", "Generate random bytes using a CSPRNG"),

	OPTION_GROUP("helper"),
	OPTION_COMMAND("help",    "Display help information about PassKeeper"),
	OPTION_COMMAND("version", "Display version information about "
				  "PassKeeper"),
	OPTION_END(),
};

static void init_enval(void)
{
	if (cred_db_path == NULL)
	{
		if ((cred_db_path = getenv(PK_CRED_DB)) == NULL)
		{
			cred_db_path = PK_CRED_DB_DEFPATH;
		}
	}

	if (cred_cc_path == NULL)
	{
		if ((cred_cc_path = getenv(PK_CRED_CC)) == NULL)
		{
			cred_cc_path = PK_CRED_CC_DEFPATH;
		}
	}

	if (tmp_rec_path == NULL)
	{
		if ((tmp_rec_path = getenv(PK_TMP_REC)) == NULL)
		{
			tmp_rec_path = PK_TMP_REC_DEFPATH;
		}
	}

	if (ext_editor != NULL);
	else if ((ext_editor = getenv(PK_EDITOR)) != NULL);
	else if ((ext_editor = getenv("VISUAL")) != NULL);
	else if ((ext_editor = getenv("EDITOR")) != NULL);
	else if ((ext_editor == NULL) && !is_dumb_term)
	{
		ext_editor = DEFAULT_EXT_EDITOR;
	}
}

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

static bool skip_precheck(int argc, const char *const *argv)
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

static void precheck_command(unsigned flags)
{
	if (flags & USE_RECFILE)
	{
		EXIT_ON_FAILURE(make_fdir_avail(tmp_rec_path), 0);
	}

	if (flags & USE_CREDFILE)
	{
		struct stat st;

		if (stat(cred_db_path, &st) != 0)
		{
			die_errno("Couldn't access file '%s'", cred_db_path);
		}
		else if (!S_ISREG(st.st_mode))
		{
			die("File '%s' is not a regular file", cred_db_path);
		}
		else if (test_file_permission(cred_db_path, &st,
						R_OK | W_OK) != 0)
		{
			die("Access denied by file '%s'", cred_db_path);
		}
	}
}

int main(int argc, const char **argv)
{
	const char *prefix;
	struct command_info *command;

	argv++;
	argc--;

	prefix = get_working_dir();

	/**
	 * turn "pk cmd --help" to "pk help cmd"
	 */
	if (argc > 1 && !strcmp(argv[1], "--help"))
	{
		argv[1] = argv[0];
		argv[0] = "help";
	}
	/**
	 * turn "pk --help" to "pk help pk"
	 */
	else if (argc == 1 && !strcmp(argv[0], "--help"))
	{
		static const char *fbcmd[] = { "help", "pk" };

		argc = 2;
		argv = fbcmd;
	}
	/**
	 * turn "pk" to "pk -h"
	 */
	else if (argc == 0)
	{
		static const char *fbcmd[] = { "-h" };

		argc = 1;
		argv = fbcmd;
	}

	optmsg_alignment = 13;

	argc = parse_options(argc, argv, prefix,
				cmd_pk_options, cmd_pk_usages,
					PARSER_UNTIL_NON_OPTION);

	optmsg_alignment = DEFAULT_OPTMSG_ALIGNMENT;

	init_enval();

	if ((command = find_command(argv[0])) == NULL)
	{
		exit(error("'%s' is not a command, see 'pk help pk'.",
				argv[0]));
	}

	/* skip command */
	argc--;
	argv++;

	if (!skip_precheck(argc, argv))
	{
		precheck_command(command->precheck_flags);
	}

	atexit(apply_atexit_chain);
	exit(command->handle(argc, argv, prefix));
}

bool have_command(const char *cmdname)
{
	return !!find_command(cmdname);
}
