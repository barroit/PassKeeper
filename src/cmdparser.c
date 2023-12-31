#include "cmdparser.h"
#include "utility.h"
#include "rescode.h"
#include "config.h"
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define OPTION_ALIAS_ID			12160
#define OPTION_ALIAS_SITENAME		12161
#define OPTION_ALIAS_SITEURL		12162
#define OPTION_ALIAS_USERNAME		12163
#define OPTION_ALIAS_PASSWORD		12164
#define OPTION_ALIAS_AUTHTEXT		12165
#define OPTION_ALIAS_BAKCODE		12166
#define OPTION_ALIAS_COMMENT		12167

app_option make_appopt(void)
{
	app_option appopt = {
		.db_pathname = getenv(DEFAULT_DB_PATHNAME + 1),
		.db_key_pathname = getenv(DEFAULT_DB_KEY_PATHNAME + 1),

		.command = NULL,

		.record_id = -1,
		.sitename = NULL,
		.siteurl = NULL,
		.username = NULL,
		.password = NULL,
		.authtext = NULL,
		.bakcode = NULL,
		.comment = NULL,

		.is_help = 0,
		.is_version = 0,
		.is_verbose = 0,
		.is_db_init = 0,

		.wrap_threshold = DEFAULT_WRAP_THRESHOLD,
	};

	return appopt;
}

int parse_cmdopts(int argc, char *argv[], app_option *appopt, const char *error_messages[2])
{
	struct option long_options[] = {
		{ "verbose",		no_argument, &appopt->is_verbose, 1 },
		{ "help",		no_argument, &appopt->is_help, 1 },
		{ "version",		no_argument, &appopt->is_version, 1 },
		{ "init",		no_argument, &appopt->is_db_init, 1 },
		{ "db",			required_argument, NULL, 'f' },
		{ "key",		required_argument, NULL, 'k' },
		{ "wrap",		required_argument, NULL, 'w' },
		{ "id",			required_argument, NULL, OPTION_ALIAS_ID },
		{ "sitename",		required_argument, NULL, OPTION_ALIAS_SITENAME },
		{ "siteurl",		required_argument, NULL, OPTION_ALIAS_SITEURL },
		{ "username",		required_argument, NULL, OPTION_ALIAS_USERNAME },
		{ "password",		required_argument, NULL, OPTION_ALIAS_PASSWORD },
		{ "authtext",		required_argument, NULL, OPTION_ALIAS_AUTHTEXT },
		{ "bakcode",		required_argument, NULL, OPTION_ALIAS_BAKCODE },
		{ "comment",		required_argument, NULL, OPTION_ALIAS_COMMENT },
		{ NULL, 0, NULL, 0 }
	};

	const char *short_options = ":vf:iw:k:"; /* leading colon make getopt quiet */

	int rc, alias, optidx;

	while (1)
	{
		if ((alias = getopt_long(argc, argv, short_options, long_options, &optidx)) == -1) /* No more options */
		{
			break;
		}

		switch (rc = handle_parse_option(appopt, alias))
		{
			case PK_SUCCESS:
				break;
			case PK_INCOMPATIBLE_TYPE:
				error_messages[ERRMSG_IDX_K] = long_options[optidx].name;
				error_messages[ERRMSG_IDX_V] = optarg;
				return rc;
			case PK_UNKNOWN_OPTION:
			case PK_MISSING_OPERAND:
				error_messages[ERRMSG_IDX_K] = argv[optind - 1];
				return rc;
			default:
				abort();
		}
	}

	return PK_SUCCESS;
}

int handle_parse_option(app_option *appopt, int alias)
{
	switch (alias)
	{
		case OPTION_ALIAS_SITENAME:
			appopt->sitename = optarg;
			break;
		case 0:
			/* flag resovled, do nothing */
			break;
		case OPTION_ALIAS_SITEURL:
			appopt->siteurl = optarg;
			break;
		case OPTION_ALIAS_USERNAME:
			appopt->username = optarg;
			break;
		case OPTION_ALIAS_PASSWORD:
			appopt->password = optarg;
			break;
		case OPTION_ALIAS_AUTHTEXT:
			appopt->authtext = optarg;
			break;
		case OPTION_ALIAS_BAKCODE:
			appopt->bakcode = optarg;
			break;
		case OPTION_ALIAS_COMMENT:
			appopt->comment = optarg;
			break;
		case OPTION_ALIAS_ID:
			if (!is_positive_integer(optarg))
			{
				return PK_INCOMPATIBLE_TYPE;
			}
			appopt->record_id = atoi(optarg);
			break;
		case 'v':
			appopt->is_verbose = 1;
			break;
		case 'f':
			appopt->db_pathname = optarg;
			break;
		case 'k':
			appopt->db_key_pathname = optarg;
			break;
		case 'i':
			appopt->is_db_init = 1;
			break;
		case 'w':
			if (!is_positive_integer(optarg))
			{
				return PK_INCOMPATIBLE_TYPE;
			}
			appopt->wrap_threshold = atoi(optarg);
			break;
		case '?':
			return PK_UNKNOWN_OPTION;
		case ':':
			return PK_MISSING_OPERAND;
		default:
			abort();
	}

	return PK_SUCCESS;
}

int parse_cmdargs(int argc, char *argv[], app_option *appopt)
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

int handle_parse_command(const char *argument, app_option *appopt)
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

int handle_parse_argument(const char *argument, app_option *appopt)
{
	switch (appopt->command[0])
	{
		case 'c':
		case 'C':
			return UNKNOW_ARGUMENT;
		case 'r':
		case 'R':
			if (appopt->sitename != NULL)
				return FIELD_CONFLICT;
			appopt->sitename = argument;
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

int validate_field(char **missing_field, const app_option *appopt)
{
	if (!is_rw_file(appopt->db_pathname))
		return FILE_INACCESS;

	switch (appopt->command[0])
	{
		case 'c':
		case 'C':
			;
			bool missing_sitename = is_empty_string(appopt->sitename);
			bool missing_username = is_empty_string(appopt->username);
			bool missing_password = is_empty_string(appopt->password);
			bool missing_credential = missing_username && missing_password;

			char *_missing_field = NULL;
			if (missing_sitename)
				_missing_field = "sitename";
			if (missing_credential)
				_missing_field = "username or password";
			if (missing_sitename && missing_credential)
				_missing_field = "sitename, username or password";

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