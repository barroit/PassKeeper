#include "cmdparser.h"
#include "utility.h"
#include "rescode.h"
#include "config.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

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

int parse_cmdopts(int argc, char *argv[], app_option *appopt, const char *errmsg[2])
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
		if ((alias = getopt_long(argc, argv, short_options, long_options, &optidx)) == -1) /* no more options */
		{
			break;
		}

		switch (rc = handle_parse_option(appopt, alias))
		{
			case PK_SUCCESS:
				break;
			case PK_INCOMPATIBLE_TYPE:
				errmsg[ERRMSG_IK] = long_options[optidx].name;
				// fall through
			case PK_UNKNOWN_OPTION:
				errmsg[ERRMSG_IV] = argv[optind - 1];
				return rc;
			case PK_MISSING_OPERAND:
				errmsg[ERRMSG_IK] = argv[optind - 1];
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

int parse_cmdargs(int argc, char *argv[], app_option *appopt, const char *errmsg[3])
{
	int rc, handind, origind; /* result code, handler index, original index */
	const char *argument;

	int (*handler[2])(const char *, app_option *) = {
		handle_parse_operation,
		handle_parse_argument,
	};

	const char *optab['u' - 'D' + 1]; /* 114 r, 82 R; 117 u, 85 U; 100 d, 68D; offset -68 */
	unsigned char margtab['u' - 'C' + 1]; /* max args lookup table */

	optab['r' - 'D'] = optab['R' - 'D'] = "sitename";
	optab['u' - 'D'] = optab['U' - 'D'] = "id";
	optab['d' - 'D'] = optab['D' - 'D'] = "id";

	margtab['c' - 'C'] = margtab['C' - 'C'] = 0;
	margtab['r' - 'C'] = margtab['R' - 'C'] = 1;
	margtab['u' - 'C'] = margtab['U' - 'C'] = 1;
	margtab['d' - 'C'] = margtab['D' - 'C'] = 1;

	origind = optind;
	while (optind < argc)
	{
		argument = argv[optind];
		handind = optind - origind;

		if (handind > 0 && handind > margtab[*appopt->command - 'C'])
		{
			errmsg[ERRMSG_IK] = argv[optind];

			if (optind + 1 < argc)
			{
				errmsg[ERRMSG_IV] = argv[optind + 1];
			}
			if (optind + 2 < argc) /* we display up to 3 arguments */
			{
				errmsg[ERRMSG_IE1] = argv[optind + 2];
			}

			return PK_TOOMANY_ARGUMENTS;
		}

		switch (rc = handler[handind](argument, appopt))
		{
			case PK_SUCCESS:
				break;
			case PK_UNCLEAR_OPTARG:
			case PK_INCOMPATIBLE_TYPE:
				errmsg[ERRMSG_IK] = optab[*appopt->command - 'D'];
				// fall through
			case PK_UNKNOWN_OPERATION:
				errmsg[ERRMSG_IV] = argument;
				return rc;
			default:
				abort();
		}

		optind++;
	}

	return PK_SUCCESS;
}

int handle_parse_operation(const char *argument, app_option *appopt)
{
	const char **iter, *commands[5] = { "create", "read", "update", "delete", NULL };

	bool is_shortcut;
	bool is_fullname;

	iter = commands;
	while (*iter != NULL)
	{
		is_shortcut = strlen(argument) == 1 && *argument == toupper(**iter);
		is_fullname = strcmp(argument, *iter) == 0;

		if (is_shortcut || is_fullname)
		{
			appopt->command = argument;
			return PK_SUCCESS;
		}

		iter++;
	}

	return PK_UNKNOWN_OPERATION;
}

int handle_parse_argument(const char *argument, app_option *appopt)
{
	switch (*appopt->command)
	{
		case 'r':
		case 'R':
			if (appopt->sitename != NULL)
			{
				return PK_UNCLEAR_OPTARG;
			}
			appopt->sitename = argument;
			break;
		case 'u':
		case 'U':
		case 'd':
		case 'D':
			if (appopt->record_id != -1)
			{
				return PK_UNCLEAR_OPTARG;
			}
			if (!is_positive_integer(argument))
			{
				return PK_INCOMPATIBLE_TYPE;
			}
			appopt->record_id = atoi(argument);
			break;
		default:
			abort();
	}

	return 0;
}

int validate_appopt(const app_option *appopt, const char *errmsg[3])
{
	if (!exists(appopt->db_pathname))
	{
		errmsg[ERRMSG_IV] = appopt->db_pathname;
		return PK_MISSING_FILE;
	}

	if (!is_rw_file(appopt->db_pathname))
	{
		errmsg[ERRMSG_IV] = appopt->db_pathname;
		return PK_PERMISSION_DENIED;
	}

	switch (*appopt->command)
	{
		case 'c':
		case 'C':
			;
			bool is_invalid_record = is_empty_string(appopt->sitename);
			bool is_useless_record = is_empty_string(appopt->username) && is_empty_string(appopt->password);

			if (is_invalid_record && is_useless_record)
			{
				errmsg[ERRMSG_IE1] = "one of the username and password";
			}

			if (is_invalid_record || is_useless_record)
			{
				errmsg[ERRMSG_IK] = appopt->command;
				errmsg[ERRMSG_IV] = is_invalid_record ? "sitename" : "one of the username and password";
				return PK_UNSATISFIED_CONDITION;
			}

			break;
		case 'u':
		case 'U':
		case 'd':
		case 'D':
			if (appopt->record_id == -1)
			{
				errmsg[ERRMSG_IK] = appopt->command;
				errmsg[ERRMSG_IV] = "sitename";
				return PK_UNSATISFIED_CONDITION;
			}
			// fall through
		case 'r':
		case 'R':
			break;
		default:
			abort();
	}

	return PK_SUCCESS;
}