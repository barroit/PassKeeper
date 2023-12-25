#include "cli.h"
#include "config.h"
#include "utility.h"
#include <string.h>

void show_version(void)
{
	printf("%s version %d.%d.%d\n"
		"Written by %s.\n",
		PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
		AUTHOR);
}

void show_all_usages(const char *appname)
{
	char *margin = strpad(strlen(appname) + 8);

	printf("usage: %s [--version] [--help] [--db_filename=<path>]\n"
		"%s<command> [<args>]\n"
		"\n"
		"There are four main commands used to interact with\n"
		"the database, use the capital letter of each command\n"
		"as an abbreviation:\n"
		"   create | C   Add new entry\n"
		"   read   | R   Retrieve entries by site name\n"
		"   update | U   Modify entry by id\n"
		"   delete | D   Remove entry by id\n",
		appname,
		margin);
	
	free(margin);
}

void show_command_usage(const char *appname, const char *command)
{
	const char *format;
	switch (command[0])
	{
		case 'c':
		case 'C':
			format = get_create_usage_format();
			break;
		case 'r':
		case 'R':
			format = get_read_usage_format();
			break;
		case 'u':
		case 'U':
			format = get_update_usage_format();
			break;
		case 'd':
		case 'D':
			format = get_delete_usage_format();
			break;
		default:
			abort();
	}

	printf(format, appname, command);
}

const char *get_create_usage_format(void)
{
	return	"usage: %s %s <options>\n"
		"\n"
		"    --sitename <name>        identify the site for which the credentials\n"
		"                              are stored, this field is required\n"
		"    --siteurl <url>          the URL of the site for visite\n"
		"    --username <name>         requires either username or password\n"
		"    --password <password>     requires either password or username\n"
		"    --authtext <text>        can include\n"
		"                              security questions, passphrase hints, or\n"
		"                              other authentication-related information\n"
		"    --bakcode <code>    this can be used for account recovery\n"
		"    --comment <comment>       include any additional comments or information\n"
		"                              related to this record\n";
}

const char *get_read_usage_format(void)
{
	return	"usage: %s %s [<options>] [sitename]\n"
		"\n"
		"    -v, --verbose             be verbose\n"
		"\n"
		"    --sitename <name>        set the name of the site, can be a\n"
		"                              positional argument\n";
}

const char *get_update_usage_format(void)
{
	return	"usage: %s %s [id] [<options>]\n"
		"\n"
		"    --id <number>             specify which record will be modified, can be\n"
		"                              a positional argument\n"
		"    --sitename <name>        site name\n"
		"    --siteurl <url>          site url\n"
		"    --username <name>         username\n"
		"    --password <password>     password\n"
		"    --authtext <text>        authentication text\n"
		"    --bakcode <code>    recovery code\n"
		"    --comment <comment>       comment\n";
}

const char *get_delete_usage_format(void)
{
	return	"usage: %s %s [id] [<options>]\n"
		"\n"
		"    --id <number>             specify which record will be deleted, can be\n"
		"                              a positional argument\n";
}