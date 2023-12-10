#include "command_parser.h"
#include "prompt.h"
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>

struct argument argument = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1, 0, 0, 0 };

static struct option long_options[] = {
	{ "verbose",		no_argument, &argument.is_verbose, 1 },
	{ "help",		no_argument, &argument.is_help, 1 },
	{ "file",		required_argument, NULL, 'f' },
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

static char *subcommands[] = { "create", "read", "update", "delete", 0 };

void parse_command(int argc, char **argv)
{
	char *short_options = "vhf:";

	int c;

	while (1)
	{
		int option_index = 0;
		c = getopt_long(argc, argv, short_options, long_options, &option_index);

		if (c == -1) // no more args
			break;

		resolve_option(c);
	}

	if (optind >= argc) // no subcommand passed
		exit(1);

	int subcommand = get_subcommand(argv);

	if (subcommand == -1) // subcommand not found
		exit(1);

	argument.subcommand = subcommand;

	int position_start = optind++;

	while (optind < argc)
	{
		int position = optind - position_start;
		resolve_pos_arg(argv[optind], position);
		optind++;
	}

	validate_argument();
}

int validate_argument(void)
{
	char *missing_field = NULL;

	switch (argument.subcommand)
	{
		case 0: // create
			if (argument.username == NULL && argument.password == NULL)
				append_string(&missing_field, "username/password");
			/* fallthrough to check to check site_name */
		case 1: // read
			if (argument.site_name == NULL)
				append_string(&missing_field, missing_field ? ", site_name" : "site_name");
			break;
		case 2: // update
		case 3: // delete
			if (argument.id == -1)
				append_string(&missing_field, "id");
			break;
		default:
			abort();
	}

	if (missing_field)
		printf("\"%s\" requires the following options: %s\n", subcommands[argument.subcommand], missing_field);

	free(missing_field);

	return 0;
}

void resolve_pos_arg(char *value, int position)
{
	/*
	for now is fine,
	we know that the amount of positional arguments
	is up to 2 (containing subcommand)
	*/
	if (position != 1)
		exit(1);

	switch (argument.subcommand)
	{
		case 1: // read
			if (argument.site_name != NULL)
				exit(1);
			argument.site_name = value;
			break;
		case 2: // update
		case 3: // delete
			if (argument.id != -1)
				exit(1);
			if (!is_number(value))
				exit(1);
			argument.id = atoi(value);
			break;
		default: // create
			exit(1);
	}
}

int get_subcommand(char **argv)
{
	char *argument = argv[optind];
	int i = 0;
	char *subcommand;
	while ((subcommand = subcommands[i]))
	{
		int argument_lenth = strlen(argument);

		int is_shortcut = argument_lenth == 1 && argument[0] == subcommand[0] - 32;
		int is_fullname = strcmp(argument, subcommand) == 0;

		if (is_shortcut || is_fullname) return i;

		i++;
	}

	return -1;
}

void resolve_option(int c)
{
	switch (c)
	{
		case OPTION_ALIAS_SITE_NAME:
			argument.site_name = optarg;
			break;
		case 0:
			break;
		case OPTION_ALIAS_SITE_URL:
			argument.site_url = optarg;
			break;
		case OPTION_ALIAS_USERNAME:
			argument.username = optarg;
			break;
		case OPTION_ALIAS_PASSWORD:
			argument.password = optarg;
			break;
		case OPTION_ALIAS_AUTH_TEXT:
			argument.auth_text = optarg;
			break;
		case OPTION_ALIAS_RECOVERY_CODE:
			argument.recovery_code = optarg;
			break;
		case OPTION_ALIAS_COMMENT:
			argument.comment = optarg;
			break;
		case OPTION_ALIAS_ID:
			if (!is_number(optarg)) exit(1); // TODO prompts
			argument.id = atoi(optarg);
			break;
		case 'v':
			argument.is_verbose = 1;
			break;
		case 'h':
			argument.is_help = 1;
			break;
		case 'f':
			argument.input_file = optarg;
			break;
		case '?':
          		/* getopt_long already printed an error message. */
			exit(1);
			break;
		default:
			exit(1);
	}
}

int is_number(char *text)
{
	while (*text)
		if (!isdigit(*text++)) return 0;

	return 1;
}

void print_arguments(void)
{
	puts("---------------- ALL FIELDS IN ARGUMENT WILL BE PRINTED ----------------");

	printf("input_file: %s\n", argument.input_file ? argument.input_file : "");

	printf("is_help: %d\n", argument.is_help);
	printf("is_verbose: %d\n", argument.is_verbose);
	printf("subcommand: %d\n", argument.subcommand);

	printf("id: %d\n", argument.id);
	printf("site_name: %s\n", argument.site_name ? argument.site_name : "");
	printf("site_url: %s\n", argument.site_url ? argument.site_url : "");
	printf("username: %s\n", argument.username ? argument.username : "");
	printf("password: %s\n", argument.password ? argument.password : "");
	printf("auth_text: %s\n", argument.auth_text ? argument.auth_text : "");
	printf("recovery_code: %s\n", argument.recovery_code ? argument.recovery_code : "");
	printf("comment: %s\n", argument.comment ? argument.comment : "");

	puts("---------------- END PRINT ----------------");
}