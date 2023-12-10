#include "command_parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>

void parse_command(int argc, char **argv, struct argument *argument)
{
	struct option long_options[] = {
		{ "verbose",		no_argument, &argument->is_verbose, 1 },
		{ "help",		no_argument, &argument->is_help, 1 },
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

	char *short_options = "vhf:";

	int c;

	while (1)
	{
		int option_index = 0;
		c = getopt_long(argc, argv, short_options, long_options, &option_index);

		// no more args
		if (c == -1) break;

		parse_options(c, argument);
	}
}

void parse_options(int c, struct argument *argument)
{
	switch (c)
	{
		case OPTION_ALIAS_SITE_NAME:
			argument->site_name = optarg;
			break;
		case 0:
			break;
		case OPTION_ALIAS_SITE_URL:
			argument->site_url = optarg;
			break;
		case OPTION_ALIAS_USERNAME:
			argument->username = optarg;
			break;
		case OPTION_ALIAS_PASSWORD:
			argument->password = optarg;
			break;
		case OPTION_ALIAS_AUTH_TEXT:
			argument->auth_text = optarg;
			break;
		case OPTION_ALIAS_RECOVERY_CODE:
			argument->recovery_code = optarg;
			break;
		case OPTION_ALIAS_COMMENT:
			argument->comment = optarg;
			break;
		case OPTION_ALIAS_ID:
			if (!is_number(optarg)) exit(1); // TODO prompts
			argument->id = atoi(optarg);
			break;
		case 'v':
			argument->is_verbose = 1;
			break;
		case 'h':
			argument->is_help = 1;
			break;
		case 'f':
			argument->input_file = optarg;
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

void print_arguments(struct argument *argument)
{
	puts("----------------ALL FIELDS IN ARGUMENT WILL BE PRINTED----------------");

	printf("input_file: %s\n", argument->input_file ? argument->input_file : "");
	printf("is_help: %d\n", argument->is_help);
	printf("is_verbose: %d\n", argument->is_verbose);

	printf("id: %d\n", argument->id);
	printf("site_name: %s\n", argument->site_name ? argument->site_name : "");
	printf("site_url: %s\n", argument->site_url ? argument->site_url : "");
	printf("username: %s\n", argument->username ? argument->username : "");
	printf("password: %s\n", argument->password ? argument->password : "");
	printf("auth_text: %s\n", argument->auth_text ? argument->auth_text : "");
	printf("recovery_code: %s\n", argument->recovery_code ? argument->recovery_code : "");
	printf("comment: %s\n", argument->comment ? argument->comment : "");

	puts("----------------END PRINT----------------");
}