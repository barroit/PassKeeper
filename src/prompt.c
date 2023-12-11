#include "prompt.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

char *app_name = NULL;
char *err_field = NULL;
char *err_msg = NULL;

void version()
{
	char *message = NULL;

	strapdf(&message, "%s version %d.%d.%d\n", PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	strapdf(&message, "Written by %s.", AUTHOR);
	puts(message);

	free(message);
	exit(EXIT_HELP);
}

void usage(int flag)
{
	char *help_message = NULL;

	switch (flag)
	{
		case USAGE_ALL:
			strapdf(&help_message, "usage: %s ", app_name);

			char *margin = NULL;
			strmgn(&margin, strlen(help_message));

			strapd(&help_message, "[--version] [--help] [--db_file=<path>]\n");
			strapdf(&help_message, "%s<command> [<args>]\n", margin);

			free(margin);

			strapd(&help_message,
				"\n"
				"There are four main commands used to interact with the database:\n"
				"   create | C   Add new entry\n"
				"   read   | R   Retrieve entries by site name\n"
				"   update | U   Modify entry by id\n"
				"   delete | D   Remove entry by id");

			break;
		case USAGE_SUBCOMMAND:
			switch (err_field[0])
			{
				case 'c':
				case 'C':
					strapdf(&help_message,
						"usage: %s %s <options>...\n"
						"\n"
						"    --site_name <name>        identify the site for which the credentials\n"
						"                              are stored, this field is required\n"
						"    --site_url <url>          the URL of the site for visite\n"
						"    --username <name>         requires either username or password\n"
						"    --password <password>     requires either password or username\n"
						"    --auth_text <text>        can include\n"
						"                              security questions, passphrase hints, or\n"
						"                              other authentication-related information\n"
						"    --recovery_code <code>    this can be used for account recovery\n"
						"    --comment <comment>       include any additional comments or information\n"
						"                              related to this record",
						app_name, err_field);
					break;
				case 'r':
				case 'R':
					strapdf(&help_message,
						"usage: %s %s [<options>] [sitename]\n"
						"\n"
						"    -v, --verbose         be verbose\n"
						"\n"
						"    --site_name <name>    set the name of the site, can be specified as a\n"
						"                          positional argument",
						app_name, err_field);
					break;
				case 'd':
				case 'D':
					strapdf(&help_message,
						"usage: %s %s [<options>] [id]\n"
						"\n"
						"    --id <number>   specify which record will be deleted, can be specified\n"
						"                    as a positional argument",
						app_name, err_field);
					break;
				case 'u':
				case 'U':
					strapdf(&help_message,
						"usage: %s %s [id] [<options>]\n"
						"\n"
						"    --id <number>             specify which record will be modified, can be\n"
						"                              specified as a positional argument\n"
						"    --site_name <name>        site name\n"
						"    --site_url <url>          site url\n"
						"    --username <name>         username\n"
						"    --password <password>     password\n"
						"    --auth_text <text>        authentication text\n"
						"    --recovery_code <code>    recovery code\n"
						"    --comment <comment>       comment",
						app_name, err_field);
					break;
				default:
					abort();
			}
			break;
		default:
			abort();
	}

	puts(help_message);

	free(help_message);
	free(err_msg);
	exit(EXIT_HELP);
}

void error(int flag)
{
	char *help_message = NULL;

	strapdf(&help_message, "%s: ", app_name);

	switch (flag)
	{
		case ERROR_UNKNOW_COMMAND:
			strapdf(&help_message, "'%s' is not a %s command. See '%s --help'.",
				err_field, PROJECT_NAME, app_name);
			break;
		case ERROR_UNKNOW_ARGUMENT:
			strapdf(&help_message, "unrecognized argument '%s'", err_field);
			break;
		case ERROR_MISSING_FIELD:
			strapdf(&help_message, "missing field '%s'", err_field ? err_field : err_msg);
			break;
		case ERROR_TYPE_MISMATCH:
			strapdf(&help_message, "value %s does not match type %s", err_field, err_msg);
			break;
		case ERROR_FIELD_CONFLICT:
			strapdf(&help_message, "reassigned field %s with value %s", err_msg, err_field);
			break;
		case ERROR_UNDEFINED_ENV:
			strapdf(&help_message, "cannot get value from environment variable name %s", err_field);
			break;
		case ERROR_FILE_INACCESSIBLE:
			switch (errno)
			{
				case ENOENT:
					strapdf(&help_message, "'%s' does not name an existing file", err_field);
					break;
				case EACCES:
					strapdf(&help_message, "the file mode of '%s' do not permit the requested access", err_field);
					break;
				default:
					strapdf(&help_message, "fail on access '%s'", err_field);
			}
			break;
		default:
			abort();
	}

	puts(help_message);

	free(help_message);
	free(err_msg);
	exit(EXIT_FAILURE);
}

int strapdf(char **str, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	int value_length = vsnprintf(NULL, 0, format, args);
	va_end(args);

	size_t str_length = *str ? strlen(*str) : 0;

	char *new_str = realloc(*str, str_length + value_length + 1);

	if (new_str == NULL)
	{
		va_end(args);
		return -1;
	}

	*str = new_str;

	va_start(args, format);
	vsnprintf(*str + str_length, value_length + 1, format, args);
	va_end(args);

	return 0;
}

int strapd(char **str, const char *value)
{
	if (value == NULL)
		return -1;

	size_t str_length = *str ? strlen(*str) : 0;
	size_t value_length = strlen(value);

	char *new_str = realloc(*str, str_length + value_length + 1);
	if (new_str == NULL)
		return -1;

	*str = new_str;

	strcpy(*str + str_length, value);

	return 0;
}

int strmgn(char **str, int margin)
{
	char* new_str = calloc(margin + 1, sizeof(char));
	if (new_str == NULL)
		return -1;

	memset(new_str, ' ', margin);
	*str = new_str;

	return 0;
}