#include "cmdparser.h"
#include "rescode.h"
#include "errhandler.h"
#include "appinfo.h"
#include "debug.h"
#include "datastore.h"
#include "utility.h"

#include <stdlib.h>

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
		exit(EXIT_SUCCESS);
	}

	if (appopt->is_db_init)
	{
	}

	if (appopt->is_db_init && optind == argc)
	{
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

	if (appopt->is_help && appopt->command != NULL)
	{
		show_command_usage(appopt->command);
		return EXIT_SUCCESS;
	}
	else if (appopt->is_help)
	{
		show_all_usages();
		return EXIT_SUCCESS;
	}

	if ((rc = validate_appopt(appopt, errmsg)) != PK_SUCCESS)
	{
		handle_validate_appopt_error(rc, errmsg);
		return EXIT_FAILURE;
	}

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

	// rc = sqlite3 _open(appopt.db_pathname, &db);

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
	return EXIT_SUCCESS;
}