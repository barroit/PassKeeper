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
char a = '\a';
#include "parse-option.h"
#include "strbuf.h"
#include "filesys.h"
#include "atexit-chain.h"
#include "command.h"

#define OPTION_FILENAME_H(s, l, v)\
	OPTION_FILENAME_F((s), (l), (v), 0, 0, OPTION_HIDDEN)

#define OPTION_STRING_H(s, l, v)\
	OPTION_STRING_F((s), (l), (v), 0, 0, OPTION_HIDDEN)

#define OPTION_OPTARG_HF(s, l, v, f)\
	OPTION_OPTARG_F((s), (l), (v), 0, 0, 0, OPTION_HIDDEN | (f))

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

static bool is_skip_precheck(int argc, const char *const *argv)
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

static void precheck_command(enum cmdreq reqs)
{
	if (reqs & USE_RECFILE)
	{
		avail_file_dir_or_die(tmp_rec_path);
	}

	if (reqs & USE_CREDDB &&
	     access_regular(cred_db_path, R_OK | W_OK) != 0)
	{
		exit(error_errno("cannot access cred db ‘%s’", cred_db_path));
	}
}

static int parse_main_option(int argc, const char **argv, const char *prefix)
{
	const struct option options[] = {
		OPTION_FILENAME_H(0, "cred-db", &cred_db_path),
		OPTION_FILENAME_H(0, "cred-cc", &cred_cc_path),
		OPTION_FILENAME_H(0, "tmp-rec", &tmp_rec_path),

		OPTION_STRING_H (0, "editor",  &ext_editor),
		OPTION_OPTARG_HF(0, "spinner", &spinner_style, OPTION_ALLONEG),

		OPTION_GROUP("database manipulation"),
		OPTION_COMMAND("init",    "Initialize database files for "
					  "storing credentials"),
		OPTION_COMMAND("create",  "Create a record"),
		OPTION_COMMAND("read",    "Read a record"),
		OPTION_COMMAND("update",  "Update a record"),
		OPTION_COMMAND("delete",  "Delete a record"),
		OPTION_COMMAND("count",   "Count the number of records"),

		OPTION_GROUP("utility"),
		OPTION_COMMAND("makekey", "Generate random bytes using "
					  "a CSPRNG"),

		OPTION_GROUP("helper"),
		OPTION_COMMAND("help",    "Display help information "
					  "about PassKeeper"),
		OPTION_COMMAND("version", "Display version information "
					  "about PassKeeper"),
		OPTION_END(),
	};

	const char *const usages[] = {
		"pk [--cred-db <file>] [--cred-cc <file>] [--temp-rc <file>]\n"
		"   [--editor <name>] [--[no]-spinner[=<style>]] <command>\n"
		"   [<args>]",
		NULL,
	};

	set_opt_argh_indent(13);
	argc = parse_options(argc, argv, prefix,
				 options, usages, PARSER_UNTIL_NON_OPTION);
	reset_opt_argh_indent();

	return argc;
}

static void help_unknown_command(const char *cmd)
{
	char **commands, **iter;

	error("‘%s’ is not a pk command, see ‘pk help pk’.", cmd);

	if ((commands = get_approximate_command(cmd)) == NULL)
	{
		goto finish;
	}

	if (commands[0] != NULL && commands[1] != NULL)
	{
		fputs("\nThe most similar command are\n", stderr);
	}
	else
	{
		fputs("\nThe most similar command is\n", stderr);
	}

	array_iterate_each(iter, commands)
	{
		fprintf(stderr, "\t%s\n", *iter);
	}

	// strarr_free(commands);

finish:
	exit(EXIT_FAILURE);
}

int main(int argc, const char **argv)
{
	const char *prefix;
	const struct cmdinfo *command;

	ARGV_MOVE_FRONT(argc, argv);

	get_working_dir(&prefix);

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
		ARGV_MOVE_BACK(argc, argv);

		argv[0] = "help";
		argv[1] = "pk";
	}
	/**
	 * turn "pk" to "pk -h"
	 */
	else if (argc == 0)
	{
		ARGV_MOVE_BACK(argc, argv);

		argv[0] = "-h";
	}

	argc = parse_main_option(argc, argv, prefix);

	init_enval();

	if ((command = find_command(argv[0])) == NULL)
	{
		help_unknown_command(argv[0]);
	}

	/* skip ‘command’ */
	ARGV_MOVE_FRONT(argc, argv);

	if (!is_skip_precheck(argc, argv))
	{
		precheck_command(command->reqs);
	}

	atexit(apply_atexit_chain);
	exit(command->handle(argc, argv, prefix));
}
