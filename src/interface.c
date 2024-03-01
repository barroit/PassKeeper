#include "cmdparser.h"
#include "rescode.h"
#include "errhandler.h"
#include "appinfo.h"
#include "debug.h"
#include "datastore.h"
#include "misc.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

extern int optind;

const char *appname;

int main(int argc, char **argv)
{
	int rc;

	const char *errmsg[4] = { NULL, NULL, NULL, NULL };

	app_option _appopt;
	app_option *appopt;

	appname = argv[0];

	_appopt = make_appopt();
	appopt = &_appopt;

	if ((rc = parse_cmdopts(argc, argv, appopt, errmsg)) != PK_SUCCESS)
	{
		handle_parse_cmdopts_error(rc, errmsg);
		return EXIT_FAILURE;
	}

	if (appopt->is_version)
	{
		show_version();
		return EXIT_SUCCESS;
	}

	bool request_encryption;
	sqlite3 *db;

	request_encryption = appopt->db_key_pathname != NULL;
	if (appopt->is_db_init)
	{
		if ((rc = init_db_file(&db, appopt->db_pathname, errmsg)) != PK_SUCCESS)
		{
			handle_init_database_error(rc, errmsg);
			return EXIT_FAILURE;
		}

		rc = PK_SUCCESS;
		if (request_encryption && (rc = encrypt_db(db, appopt->db_key_pathname, errmsg)) != PK_SUCCESS)
		{
			handle_db_key_error(rc, errmsg);
		}

		if (rc == PK_SUCCESS && (rc = init_db_table(db)) != SQLITE_OK)
		{
			handle_sqlite_error(rc);
		}

		sqlite3_close(db);

		if (rc != SQLITE_OK)
		{
			return EXIT_FAILURE;
		}
	}

	if (appopt->is_db_init && optind == argc)
	{
		printf("table 'account' created in database '%s'\n", appopt->db_pathname);
		return EXIT_SUCCESS;
	}

	if (optind == argc && !appopt->is_help) /* no operation found */
	{
		handle_missing_operation_error();
		return EXIT_FAILURE;
	}

	if ((rc = parse_cmdargs(argc, argv, appopt, errmsg)) != PK_SUCCESS)
	{
		handle_parse_cmdargs_error(rc, errmsg);
		return EXIT_FAILURE;
	}

	if (appopt->is_help)
	{
		if (appopt->command != NULL)
		{
			show_command_usage(appopt->command);
		}
		else
		{
			show_all_usages();
		}

		return EXIT_SUCCESS;
	}

	if ((rc = validate_appopt(appopt, errmsg)) != PK_SUCCESS)
	{
		handle_validate_appopt_error(rc, errmsg);
		return EXIT_FAILURE;
	}

	debug_log("db_pathname: %s\n"
		"db_key_pathname: %s\n"
		"command: %s\n"
		"is_help: %s\n"
		"is_version: %s\n"
		"is_verbose: %s\n"
		"is_db_init: %s\n"
		"record_id: %d\n"
		"sitename: %s\n"
		"siteurl: %s\n"
		"username: %s\n"
		"password: %s\n"
		"authtext: %s\n"
		"bakcode: %s\n"
		"comment: %s\n",
		STRINGIFY(appopt->db_pathname),
		STRINGIFY(appopt->db_key_pathname),
		STRINGIFY(appopt->command),
		STRBOOL(appopt->is_help),
		STRBOOL(appopt->is_version),
		STRBOOL(appopt->is_verbose),
		STRBOOL(appopt->is_db_init),
		appopt->record_id,
		STRINGIFY(appopt->sitename),
		STRINGIFY(appopt->siteurl),
		STRINGIFY(appopt->username),
		STRINGIFY(appopt->password),
		STRINGIFY(appopt->authtext),
		STRINGIFY(appopt->bakcode),
		STRINGIFY(appopt->comment));

	if ((rc = sqlite3_open(appopt->db_pathname, &db)) != SQLITE_OK)
	{
		handle_sqlite_error(rc);
		return EXIT_FAILURE;
	}

	if (request_encryption)
	{
		if ((rc = decrypt_db(db, appopt->db_key_pathname, errmsg)) != PK_SUCCESS)
		{
			handle_db_key_error(rc, errmsg);
		}

		if (rc == PK_SUCCESS && (rc = !is_db_decrypted(db)) == 1)
		{
			handle_db_key_error(PK_INCORRECT_KEY, NULL);
		}

		if (rc != PK_SUCCESS)
		{
			sqlite3_close(db);
			return EXIT_FAILURE;
		}
	}

	char *message;
	int (*cmdfn_table['u' - 'C' + 1])(sqlite3 *, const app_option *, char **message);

	cmdfn_table['c' - 'C'] = cmdfn_table['C' - 'C'] = create_record;
	cmdfn_table['r' - 'C'] = cmdfn_table['R' - 'C'] = read_record;
	cmdfn_table['u' - 'C'] = cmdfn_table['U' - 'C'] = update_record;
	cmdfn_table['d' - 'C'] = cmdfn_table['D' - 'C'] = delete_record;

	message = NULL;
	if ((rc = cmdfn_table[*appopt->command - 'C'](db, appopt, &message)) != SQLITE_OK)
	{
		handle_sqlite_error(rc);
	}

	if (rc == SQLITE_OK && *appopt->command != 'r' && *appopt->command != 'R')
	{
		if (message == NULL)
		{
			printf("nothing to be done for %s\n", appopt->command);
		}
		else
		{
			printf("a record for '%s' has been %sd\n", message, appopt->command);
		}
	}

	if (message != NULL)
	{
		free(message);
	}

	sqlite3_close(db);

	return rc == SQLITE_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}