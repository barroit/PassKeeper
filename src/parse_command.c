#include "parse_command.h"
#include "utility.h"
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define OPTION_ALIAS_ID			12160
#define OPTION_ALIAS_SITE_NAME		12161
#define OPTION_ALIAS_SITE_URL		12162
#define OPTION_ALIAS_USERNAME		12163
#define OPTION_ALIAS_PASSWORD		12164
#define OPTION_ALIAS_AUTH_TEXT		12165
#define OPTION_ALIAS_RECOVERY_CODE	12166
#define OPTION_ALIAS_COMMENT		12167

int parse_cmd_opts(int argc, char *argv[], struct app_option *appopt)
{
	struct option long_options[] = {
		{ "verbose",		no_argument, &appopt->is_verbose, 1 },
		{ "help",		no_argument, &appopt->is_help, 1 },
		{ "version",		no_argument, &appopt->is_version, 1 },
		{ "db_file",		required_argument, NULL, 'f' },
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

	char *short_options = "vf:";

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
					return ERR_PARSING_INTEGER;
				appopt->record_id = atoi(optarg);
				break;
			case 'v':
				appopt->is_verbose = 1;
				break;
			case 'f':
				appopt->db_file = optarg;
				break;
			case '?':
				/* getopt_long already print an error message */
				return ERR_UNKNOW_OPT;
			default:
				abort();
		}
	}

	return 0;
}

int parse_cmd_args(int argc, char *argv[], struct app_option *appopt)
{
	char *arg;
	int result;
	int arg_start = optind;

	while (optind < argc)
	{
		arg = argv[optind];

		switch (optind - arg_start)
		{
			case 0:
				if ((result = handle_command_parse(arg, appopt)) != 0)
					return ERR_PARSING_COMMAND;
				break;
			case 1:
				if ((result = handle_argument_parse(arg, appopt)) != 0)
					return result;
				break;
			default:
				return ERR_UNKNOW_ARGUMENT;
		}

		optind++;
	}

	return 0;
}

int handle_command_parse(const char *argument, struct app_option *appopt)
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

	return ERR_PARSING_COMMAND;
}

int handle_argument_parse(const char *argument, struct app_option *appopt)
{
	switch (appopt->command[0])
	{
		case 'c':
		case 'C':
			return ERR_UNKNOW_ARGUMENT;
		case 'r':
		case 'R':
			if (appopt->site_name != NULL)
				return ERR_APPOPT_CONFLICT;
			appopt->site_name = argument;
			break;
		case 'u':
		case 'U':
		case 'd':
		case 'D':
			if (appopt->record_id != -1)
				return ERR_APPOPT_CONFLICT;
			if (!is_positive_integer(argument))
				return ERR_PARSING_INTEGER;
			appopt->record_id = atoi(argument);
			break;
		default:
			abort();
	}

	return 0;
}

int validate_field(char **missing_field, const struct app_option *appopt)
{
	if (!is_rw_file(appopt->db_file))
		return ERR_FILE_INACCESS;

	switch (appopt->command[0])
	{
		case 'c':
		case 'C':
			;
			bool missing_site_name = is_empty_string(appopt->site_name);
			bool missing_username = strcmp(appopt->username, "NULL") == 0 || appopt->username[0] == '\0';
			bool missing_password = strcmp(appopt->password, "NULL") == 0 || appopt->password[0] == '\0';
			bool missing_credential = missing_username && missing_password;

			if (missing_site_name)
				*missing_field = "site_name";
			if (missing_credential)
				*missing_field = "username or password";
			if (missing_site_name && missing_credential)
				*missing_field = "site_name, username or password";

			if (*missing_field)
				return ERR_MISSING_FIELD;
			break;
		case 'r':
		case 'R':
			if (is_empty_string(appopt->site_name))
			{
				*missing_field = "site_name";
				return ERR_MISSING_FIELD;
			}
			break;
		case 'u':
		case 'U':
		case 'd':
		case 'D':
			if (appopt->record_id == -1)
			{
				*missing_field = "id";
				return ERR_MISSING_FIELD;
			}
			break;
		default:
			abort();
	}

	return 0;
}