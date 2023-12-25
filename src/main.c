#include "cmdparser.h"
#include "debug.h"
#include "cli.h"
#include "utility.h"
#include "datastore.h"
#include <getopt.h>

int main(int argc, char **argv)
{
	int rc;
	char *message;
	sqlite3 *db;

	char *appname = argv[0];
	struct app_option appopt = get_appopt();

	rc = parse_cmdopts(argc, argv, &appopt);

	switch (rc)
	{
		case 0:
			break;
		case NOT_INTEGER:
			fatal(EXIT_FAILURE, "unable to convert value '%s' to integer id", appname, optarg);
			// fall through
		case UNKNOW_OPTION:
			exit(EXIT_FAILURE);
		default:
			abort();
	}

	if (appopt.is_version)
	{
		show_version();
		exit(EXIT_PROMPT);
	}

	if (appopt.is_db_init)
	{
		rc = init_db_file(appopt.db_filename, &db);

		switch (rc)
		{
			case 0:
				break;
			case FILENAME_EXISTS:
				fatal(EXIT_FAILURE, "'%s' already exists", appname, appopt.db_filename);
			case INVALID_FILENAME:
				fatal(EXIT_FAILURE, "invalid filename '%s'", appname, appopt.db_filename);
			case MKDIR_FAILURE:
				fatal(EXIT_FAILURE, "unable to make parent directory of '%s'", appname, appopt.db_filename);
			default:
				fatal(EXIT_FAILURE, "unable to open database, %s", appname, sqlite3_errstr(rc));
		}

		rc = init_db_table(db);

		if (rc != SQLITE_OK)
			REPORT_ERROR("unable to create table 'account', %s", appname, sqlite3_errstr(rc));

		rc = sqlite3_close(db);

		if (rc != SQLITE_OK)
			fatal(EXIT_FAILURE, "unable to close db connection, %s", appname, sqlite3_errstr(rc));

		printf("table 'account' created in scheme '%s'\n", appopt.db_filename);
	}

	if (appopt.is_db_init && optind == argc)
		return EXIT_SUCCESS;

	/* no argument found */
	if (optind == argc)
	{
		show_all_usages(appname);
		exit(EXIT_PROMPT);
	}

	rc = parse_cmdargs(argc, argv, &appopt);

	switch (rc)
	{
		case 0:
			break;
		case COMMAND_MISMATCH:
			fatal(EXIT_FAILURE, "'%s' is not a valid command", appname, argv[optind]);
		case UNKNOW_ARGUMENT:
			fatal(EXIT_FAILURE, "unknown argument '%s'", appname, argv[optind]);
		case NOT_INTEGER:
			fatal(EXIT_FAILURE, "unable to convert value '%s' to integer id", appname, argv[optind]);
		case FIELD_CONFLICT:
			fatal(EXIT_FAILURE, "argument value '%s' conflicted with option value", appname, argv[optind]);
		default:
			abort();
	}

	if (appopt.is_help)
	{
		show_command_usage(appname, appopt.command);
		exit(EXIT_PROMPT);
	}

	rc = validate_field(&message, &appopt);

	switch (rc)
	{
		case 0:
			break;
		case FILE_INACCESS:
			fatal(EXIT_FAILURE, "db file '%s' is not meets the requirement -rw-...", appname, PRINTABLE_STRING(appopt.db_filename));
		case MISSING_FIELD:
			fatal(EXIT_FAILURE, "missing field '%s'", appname, message);
		default:
			abort();
	}

	debug_message(print_appopt(&appopt));

	rc = sqlite3_open(appopt.db_filename, &db);

	if (rc != SQLITE_OK)
		fatal(EXIT_FAILURE, "unable to open database, %s", appname, sqlite3_errstr(rc));

	switch (appopt.command[0])
	{
		case 'c':
		case 'C':
			rc = create_record(db, &appopt);
			break;
		case 'r':
		case 'R':
			rc = read_record(db, &appopt);
			break;
		case 'u':
		case 'U':
			break;
		case 'd':
		case 'D':
			break;
		default:
			abort();
	}

	if (rc != SQLITE_OK)
		REPORT_ERROR("unable to execute statement, %s", appname, sqlite3_errstr(rc));

	rc = sqlite3_close(db);

	if (rc != SQLITE_OK)
		fatal(EXIT_FAILURE, "unable to close db connection, %s", appname, sqlite3_errstr(rc));

	return rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}