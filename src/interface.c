#include "cmdparser.h"
#include "debug.h"
#include "cli.h"
#include "utility.h"
#include "datastore.h"
#include "rescode.h"
#include "errhandler.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
	int rc;

	const char *error_messages[4];

	app_option _appopt;
	app_option *appopt;

	_appopt = make_appopt();
	appopt = &_appopt;

	if ((rc = parse_cmdopts(argc, argv, appopt, error_messages)) != PK_SUCCESS)
	{
		handle_parse_cmdopts_error(rc, error_messages);
		return EXIT_FAILURE;
	}

	// switch (rc)
	// {
	// 	case 0:
	// 		break;
	// 	case NOT_INTEGER:
	// 		PRINT_ERROR("unable to convert value '%s' to integer", optarg);
	// 		// fall through
	// 	case UNKNOW_OPTION:
	// 		return EXIT_FAILURE;
	// 	default:
	// 		abort();
	// }

	// if (appopt.is_version)
	// {
	// 	show_version();
	// 	exit(EXIT_PROMPT);
	// }

	// if (appopt.is_db_init)
	// {
	// 	rc = make_db_dir(appopt.db_pathname);
	// 	switch (rc)
	// 	{
	// 		case PK_SUCCESS:
	// 			break;
	// 		case PK_FILE_EXIST:
	// 			fatal(EXIT_FAILURE, "'%s' already exists", appname, appopt.db_pathname);
	// 		case PK_INVALID_PATHNAME:
	// 			fatal(EXIT_FAILURE, "invalid filename '%s'", appname, appopt.db_pathname);
	// 		case PK_MKDIR_FAILURE:
	// 			fatal(EXIT_FAILURE, "unable to make parent directory of '%s'", appname, appopt.db_pathname);
	// 		default:
	// 			abort();
	// 	}

	// 	if ((rc = sqlite3_open(appopt.db_pathname, &db)) != SQLITE_OK)
	// 	{
	// 		fatal(EXIT_FAILURE, "unable to open database, %s", appname, sqlite3_errstr(rc));
	// 	}

	// 	if ((rc = create_db_table(db)) != SQLITE_OK)
	// 	{
	// 		REPORT_ERROR("unable to create table 'account', %s", appname, sqlite3_errstr(rc));
	// 	}

	// 	if ((rc = sqlite3_close(db)) != SQLITE_OK)
	// 	{
	// 		fatal(EXIT_FAILURE, "unable to close db connection, %s", appname, sqlite3_errstr(rc));
	// 	}

	// 	printf("table 'account' created in scheme '%s'\n", appopt.db_pathname);
	// }

	// if (appopt.is_db_init && optind == argc)
	// 	return EXIT_SUCCESS;

	// /* no argument found */
	// if (optind == argc)
	// {
	// 	show_all_usages(appname);
	// 	exit(EXIT_PROMPT);
	// }

	// rc = parse_cmdargs(argc, argv, &appopt);

	// switch (rc)
	// {
	// 	case 0:
	// 		break;
	// 	case COMMAND_MISMATCH:
	// 		fatal(EXIT_FAILURE, "'%s' is not a valid command", appname, argv[optind]);
	// 	case UNKNOW_ARGUMENT:
	// 		fatal(EXIT_FAILURE, "unknown argument '%s'", appname, argv[optind]);
	// 	case NOT_INTEGER:
	// 		fatal(EXIT_FAILURE, "unable to convert value '%s' to integer id", appname, argv[optind]);
	// 	case FIELD_CONFLICT:
	// 		fatal(EXIT_FAILURE, "argument value '%s' conflicted with option value", appname, argv[optind]);
	// 	default:
	// 		abort();
	// }

	// if (appopt.is_help)
	// {
	// 	show_command_usage(appname, appopt.command);
	// 	exit(EXIT_PROMPT);
	// }

	// rc = validate_field(&message, &appopt);

	// switch (rc)
	// {
	// 	case 0:
	// 		break;
	// 	case FILE_INACCESS:
	// 		fatal(EXIT_FAILURE, "db file '%s' is not meets the requirement -rw-...", appname, STRINGIFY(appopt.db_pathname));
	// 	case MISSING_FIELD:
	// 		fatal(EXIT_FAILURE, "missing field '%s'", appname, message);
	// 	default:
	// 		abort();
	// }

	// debug_log("db_pathname: %s\n"
	// 	"db_key_pathname: %s\n"
	// 	"command: %s\n"
	// 	"is_help: %s\n"
	// 	"is_version: %s\n"
	// 	"is_verbose: %s\n"
	// 	"is_db_init: %s\n"
	// 	"record_id: %d\n"
	// 	"sitename: %s\n"
	// 	"siteurl: %s\n"
	// 	"username: %s\n"
	// 	"password: %s\n"
	// 	"authtext: %s\n"
	// 	"bakcode: %s\n"
	// 	"comment: %s\n",
	// 	STRINGIFY(appopt.db_pathname),
	// 	STRINGIFY(appopt.db_key_pathname),
	// 	STRINGIFY(appopt.command),
	// 	STRBOOL(appopt.is_help),
	// 	STRBOOL(appopt.is_version),
	// 	STRBOOL(appopt.is_verbose),
	// 	STRBOOL(appopt.is_db_init),
	// 	appopt.record_id,
	// 	STRINGIFY(appopt.sitename),
	// 	STRINGIFY(appopt.siteurl),
	// 	STRINGIFY(appopt.username),
	// 	STRINGIFY(appopt.password),
	// 	STRINGIFY(appopt.authtext),
	// 	STRINGIFY(appopt.bakcode),
	// 	STRINGIFY(appopt.comment));

	// rc = sqlite3_open(appopt.db_pathname, &db);

	// apply_db_key(db, "");

	// if (!is_db_decrypted(db))
	// {
	// 	rc = sqlite3_close(db);
	// 	fputs("test\n", stderr);
	// 	exit(1);
	// }

	// if (rc != SQLITE_OK)
	// 	fatal(EXIT_FAILURE, "unable to open database, %s", appname, sqlite3_errstr(rc));

	// switch (appopt.command[0])
	// {
	// 	case 'c':
	// 	case 'C':
	// 		rc = create_record(db, &appopt);
	// 		break;
	// 	case 'r':
	// 	case 'R':
	// 		rc = read_record(db, &appopt);
	// 		break;
	// 	case 'u':
	// 	case 'U':
	// 		break;
	// 	case 'd':
	// 	case 'D':
	// 		break;
	// 	default:
	// 		abort();
	// }

	// if (rc != SQLITE_OK)
	// 	REPORT_ERROR("unable to execute statement, %s", appname, sqlite3_errstr(rc));

	// rc = sqlite3_close(db);

	// if (rc != SQLITE_OK)
	// 	fatal(EXIT_FAILURE, "unable to close db connection, %s", appname, sqlite3_errstr(rc));

	// return rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
	return 0;
}