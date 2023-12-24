#include "cmdparser.h"
#include "utility.h"
#include "config.h"
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define WRAP_THRESHOLD 24

#define OPTION_ALIAS_ID			12160
#define OPTION_ALIAS_SITE_NAME		12161
#define OPTION_ALIAS_SITE_URL		12162
#define OPTION_ALIAS_USERNAME		12163
#define OPTION_ALIAS_PASSWORD		12164
#define OPTION_ALIAS_AUTH_TEXT		12165
#define OPTION_ALIAS_RECOVERY_CODE	12166
#define OPTION_ALIAS_COMMENT		12167

struct app_option get_appopt(void)
{
	struct app_option appopt;

	appopt.db_filename = getenv(DEFAULT_DB_FILENAME + 1);

	appopt.command = NULL;

	appopt.record_id = -1;
	appopt.site_name = NULL;
	appopt.site_url = NULL;
	appopt.username = NULL;
	appopt.password = NULL;
	appopt.auth_text = NULL;
	appopt.recovery_code = NULL;
	appopt.comment = NULL;

	appopt.is_help = 0;
	appopt.is_version = 0;
	appopt.is_verbose = 0;
	appopt.is_db_init = 0;

	appopt.wrap_threshold = WRAP_THRESHOLD;

	return appopt;
}

int parse_cmdopts(int argc, char *argv[], struct app_option *appopt)
{
	struct option long_options[] = {
		{ "verbose",		no_argument, &appopt->is_verbose, 1 },
		{ "help",		no_argument, &appopt->is_help, 1 },
		{ "version",		no_argument, &appopt->is_version, 1 },
		{ "init",		no_argument, &appopt->is_db_init, 1 },
		{ "db",			required_argument, NULL, 'f' },
		{ "wrap",		required_argument, NULL, 'w' },
		{ "id",			required_argument, NULL, OPTION_ALIAS_ID },
		{ "site_name",		required_argument, NULL, OPTION_ALIAS_SITE_NAME },
		{ "site_url",		required_argument, NULL, OPTION_ALIAS_SITE_URL },
		{ "username",		required_argument, NULL, OPTION_ALIAS_USERNAME },
		{ "password",		required_argument, NULL, OPTION_ALIAS_PASSWORD },
		{ "auth_text",		required_argument, NULL, OPTION_ALIAS_AUTH_TEXT },
		{ "recovery_code",	required_argument, NULL, OPTION_ALIAS_RECOVERY_CODE },
		{ "comment",		required_argument, NULL, OPTION_ALIAS_COMMENT },
		{ NULL, 0, NULL, 0 }
	};

	char *short_options = "vf:iw:";

	int c;
	int option_index = 0;

	while (1)
	{
		c = getopt_long(argc, argv, short_options, long_options, &option_index);

		if (c == -1)
			break;

		switch (c)
		{
			case OPTION_ALIAS_SITE_NAME:
				appopt->site_name = optarg;
				break;
			case 0:
				/* resovle flag, do nothing */
				break;
			case OPTION_ALIAS_SITE_URL:
				appopt->site_url = optarg;
				break;
			case OPTION_ALIAS_USERNAME:
				appopt->username = optarg;
				break;
			case OPTION_ALIAS_PASSWORD:
				appopt->password = optarg;
				break;
			case OPTION_ALIAS_AUTH_TEXT:
				appopt->auth_text = optarg;
				break;
			case OPTION_ALIAS_RECOVERY_CODE:
				appopt->recovery_code = optarg;
				break;
			case OPTION_ALIAS_COMMENT:
				appopt->comment = optarg;
				break;
			case OPTION_ALIAS_ID:
				if (!is_positive_integer(optarg))
					return NOT_INTEGER;
				appopt->record_id = atoi(optarg);
				break;
			case 'v':
				appopt->is_verbose = 1;
				break;
			case 'f':
				appopt->db_filename = optarg;
				break;
			case 'i':
				appopt->is_db_init = 1;
				break;
			case 'w':
				if (!is_positive_integer(optarg))
					return NOT_INTEGER;
				appopt->wrap_threshold = atoi(optarg);
				break;
			case '?':
				/* getopt_long already print an error message */
				return UNKNOW_OPTION;
			default:
				abort();
		}
	}

	return 0;
}

int parse_cmdargs(int argc, char *argv[], struct app_option *appopt)
{
	char *arg;
	int rc;
	int arg_start = optind;

	while (optind < argc)
	{
		arg = argv[optind];

		switch (optind - arg_start)
		{
			case 0:
				if ((rc = handle_parse_command(arg, appopt)) != 0)
					return rc;
				break;
			case 1:
				if ((rc = handle_parse_argument(arg, appopt)) != 0)
					return rc;
				break;
			default:
				return UNKNOW_ARGUMENT;
		}

		optind++;
	}

	return 0;
}

int handle_parse_command(const char *argument, struct app_option *appopt)
{
	char *commands[] = { "create", "read", "update", "delete", NULL };

	int idx = 0;
	char *command;
	while ((command = commands[idx]) != NULL)
	{
		int is_shortcut = strlen(argument) == 1 && argument[0] == toupper(command[0]);
		int is_fullname = strcmp(argument, command) == 0;

		if (is_shortcut || is_fullname)
		{
			appopt->command = argument;
			return 0;
		}

		idx++;
	}

	return COMMAND_MISMATCH;
}

int handle_parse_argument(const char *argument, struct app_option *appopt)
{
	switch (appopt->command[0])
	{
		case 'c':
		case 'C':
			return UNKNOW_ARGUMENT;
		case 'r':
		case 'R':
			if (appopt->site_name != NULL)
				return FIELD_CONFLICT;
			appopt->site_name = argument;
			break;
		case 'u':
		case 'U':
		case 'd':
		case 'D':
			if (appopt->record_id != -1)
				return FIELD_CONFLICT;
			if (!is_positive_integer(argument))
				return NOT_INTEGER;
			appopt->record_id = atoi(argument);
			break;
		default:
			abort();
	}

	return 0;
}

int validate_field(char **missing_field, const struct app_option *appopt)
{
	if (!is_rw_file(appopt->db_filename))
		return FILE_INACCESS;

	switch (appopt->command[0])
	{
		case 'c':
		case 'C':
			;
			bool missing_site_name = is_empty_string(appopt->site_name);
			bool missing_username = is_empty_string(appopt->username);
			bool missing_password = is_empty_string(appopt->password);
			bool missing_credential = missing_username && missing_password;

			char *_missing_field = NULL;
			if (missing_site_name)
				_missing_field = "site_name";
			if (missing_credential)
				_missing_field = "username or password";
			if (missing_site_name && missing_credential)
				_missing_field = "site_name, username or password";

			if (_missing_field)
			{
				*missing_field = _missing_field;
				return MISSING_FIELD;
			}
			break;
		case 'r':
		case 'R':
			break;
		case 'u':
		case 'U':
		case 'd':
		case 'D':
			if (appopt->record_id == -1)
			{
				*missing_field = "id";
				return MISSING_FIELD;
			}
			break;
		default:
			abort();
	}

	return 0;
}