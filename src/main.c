#include "cmdparser.h"
#include "debug.h"
#include "cli.h"
#include "utility.h"
#include "datastore.h"
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

int main(int argc, char **argv)
{
	int result;
	char *message = NULL;

	char *appname = argv[0];
	struct app_option appopt = get_appopt();

	result = parse_cmdopts(argc, argv, &appopt);

	switch (result)
	{
		case 0:
			break;
		case ERR_PARSING_INTEGER:
			report_error("error while parsing id, value '%s' is not an integer", appname, optarg);
			// fall through
		case ERR_UNKNOW_OPT:
			exit(EXIT_PROMPT);
		default:
			abort();
	}

	if (appopt.is_version)
	{
		show_version();
		exit(EXIT_PROMPT);
	}

	/* no argument found */
	if (optind == argc)
	{
		show_all_usages(appname);
		exit(EXIT_PROMPT);
	}

	result = parse_cmdargs(argc, argv, &appopt);

	switch (result)
	{
		case 0:
			break;
		case ERR_PARSING_COMMAND:
			report_error("'%s' is not a command", appname, argv[optind]);
			exit(EXIT_PROMPT);
		case ERR_UNKNOW_ARGUMENT:
			report_error("unknown argument '%s'", appname, argv[optind]);
			exit(EXIT_PROMPT);
		case ERR_PARSING_INTEGER:
			report_error("error while parsing id, value '%s' is not an integer", appname, argv[optind]);
			exit(EXIT_PROMPT);
		case ERR_APPOPT_CONFLICT:
			report_error("argument value '%s' conflicted with option value", appname, argv[optind]);
			exit(EXIT_PROMPT);
		default:
			abort();
	}

	if (appopt.is_help)
	{
		show_command_usage(appname, appopt.command);
		exit(EXIT_PROMPT);
	}

	result = validate_field(&message, &appopt);

	switch (result)
	{
		case 0:
			break;
		case ERR_FILE_INACCESS:
			report_error("db file '%s' does not meet the requirements (-rw...)", appname, PRINTABLE_STRING(appopt.db_filename));
			exit(EXIT_PROMPT);
		case ERR_MISSING_FIELD:
			report_error("missing field '%s'", appname, message);
			exit(EXIT_PROMPT);
		default:
			abort();
	}

	DEBUG_ONLY(print_appopt(&appopt));

	return EXIT_SUCCESS;
}