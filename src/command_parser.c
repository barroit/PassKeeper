#include "command_parser.h"
#include "prompt.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

struct field field = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1, 0, 0, 0, 0 };

static char *command_list[] = { "create", "read", "update", "delete", NULL };

void parse_command(int argc, char **argv)
{
	struct option long_options[] = {
		{ "verbose",		no_argument, &field.is_verbose, 1 },
		{ "help",		no_argument, &field.is_help, 1 },
		{ "version",		no_argument, &field.is_version, 1 },
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
	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
		resolve_option(c);

	if (field.is_version)
		version();

	/* no argument found */
	if (optind >= argc)
		usage(USAGE_ALL);

	int subcommand = get_command(argv);

	/* request help */
	if (field.is_help && subcommand == -1)
		usage(USAGE_ALL);

	if (subcommand == -1)
	{
		err_field = argv[optind];
		error(ERROR_UNKNOW_COMMAND);
	}

	/* request help for command */
	if (field.is_help && subcommand + 1)
	{
		err_field = argv[optind];
		usage(USAGE_SUBCOMMAND);
	}

	field.subcommand = subcommand;

	int position_start = optind++;

	while (optind < argc)
	{
		int position = optind - position_start; // count arguments
		resolve_argument(argv[optind], position);
		optind++;
	}

	if (field.db_file == NULL)
		field.db_file = DEFAULT_DB_FILE;
}

void resolve_option(int c)
{
	switch (c)
	{
		case OPTION_ALIAS_SITE_NAME:
			field.site_name = optarg;
			break;
		case 0:
			/* resovle flag, do nothing */
			break;
		case OPTION_ALIAS_SITE_URL:
			field.site_url = optarg;
			break;
		case OPTION_ALIAS_USERNAME:
			field.username = optarg;
			break;
		case OPTION_ALIAS_PASSWORD:
			field.password = optarg;
			break;
		case OPTION_ALIAS_AUTH_TEXT:
			field.auth_text = optarg;
			break;
		case OPTION_ALIAS_RECOVERY_CODE:
			field.recovery_code = optarg;
			break;
		case OPTION_ALIAS_COMMENT:
			field.comment = optarg;
			break;
		case OPTION_ALIAS_ID:
			if (!is_number(optarg))
			{
				err_field = optarg;
				strapd(&err_msg, "int");
				error(ERROR_TYPE_MISMATCH);
			}
			field.id = atoi(optarg);
			break;
		case 'v':
			field.is_verbose = 1;
			break;
		case 'f':
			field.db_file = optarg;
			break;
		case '?':
          		/* getopt_long already printed an error message. */
			exit(1);
			break;
		default:
			abort();
	}
}

int get_command(char **argv)
{
	int i = 0;
	char *command;
	while ((command = command_list[i]))
	{
		int argument_length = strlen(argv[optind]);

		int is_shortcut = argument_length == 1 && argv[optind][0] == toupper(command[0]);
		int is_fullname = strcmp(argv[optind], command) == 0;

		if (is_shortcut || is_fullname)
			return i;

		i++;
	}

	return -1;
}

void resolve_argument(char *value, int position)
{
	/* for now is fine, we know that the amount of argument is up to 2 (containing command) */
	if (position != 1)
	{
		err_field = value;
		error(ERROR_UNKNOW_ARGUMENT);
	}

	switch (field.subcommand)
	{
		case 0: // create
			err_field = value;
			error(ERROR_UNKNOW_ARGUMENT);
			break;
		case 1: // read
			/* disallow argument and option from appearing at the same time to avoid confusion */
			if (field.site_name != NULL)
			{
				err_field = value;
				strapd(&err_msg, "site_name");
				error(ERROR_FIELD_CONFLICT);
			}
			field.site_name = value;
			break;
		case 2: // update
		case 3: // delete
			if (field.id != -1)
			{
				err_field = value;
				strapd(&err_msg, "id");
				error(ERROR_FIELD_CONFLICT);
			}
			if (!is_number(value))
			{
				err_field = value;
				strapd(&err_msg, "int");
				error(ERROR_TYPE_MISMATCH);
			}
			field.id = atoi(value);
			break;
		default:
			abort();
	}
}

void validate_field(void)
{
	/* no db file specified, use environment variable name */
	if (field.db_file[0] == '$')
	{
		char *db_file;
		if ((db_file = getenv(field.db_file + 1)) == NULL)
		{
			err_field = field.db_file;
			error(ERROR_UNDEFINED_ENV);
		}
		field.db_file = db_file;
	}

	if (access(field.db_file, F_OK | R_OK | W_OK) != 0)
	{
		err_field = field.db_file;
		error(ERROR_FILE_INACCESSIBLE);
	}

	switch (field.subcommand)
	{
		case 0: // create
			if (field.site_name == NULL)
			{
				strapd(&err_msg, "site_name");
			}
			if (field.username == NULL && field.password == NULL)
			{
				if (err_msg)
					strapd(&err_msg, ", ");
				strapd(&err_msg, "username or password");
			}
			if (err_msg)
				error(ERROR_MISSING_FIELD);
			break;
		case 1: // read
			if (field.site_name == NULL)
			{
				err_field = "site_name";
				error(ERROR_MISSING_FIELD);
			}
			break;
		case 2: // update
		case 3: // delete
			if (field.id == -1)
			{
				err_field = "id";
				error(ERROR_MISSING_FIELD);
			}
			break;
		default:
			abort();
	}
}

int is_number(char *text)
{
	while (*text)
		if (!isdigit(*text++)) return 0;

	return 1;
}

void print_field(void)
{
	char *message = NULL;

	strapdf(&message,
		"---------------- START PRINT ----------------\n"
		"db_file: %s\n"
		"is_help: %s\n"
		"is_verbose: %s\n"
		"subcommand: %s\n"
		"id: %d\n"
		"site_name: %s\n"
		"site_url: %s\n"
		"username: %s\n"
		"password: %s\n"
		"auth_text: %s\n"
		"recovery_code: %s\n"
		"comment: %s\n"
		"---------------- END PRINT ----------------",
		SAFE_STRING(field.db_file),
		TO_BOOL(field.is_help),
		TO_BOOL(field.is_verbose),
		command_list[field.subcommand],
		field.id,
		SAFE_STRING(field.site_name),
		SAFE_STRING(field.site_url),
		SAFE_STRING(field.username),
		SAFE_STRING(field.password),
		SAFE_STRING(field.auth_text),
		SAFE_STRING(field.recovery_code),
		SAFE_STRING(field.comment));

	puts(message);
	free(message);
}