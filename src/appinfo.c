#include "appinfo.h"
#include "config.h"
#include "strutil.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern const char *appname;

void show_version(void)
{
	printf("%s version %d.%d.%d\n"
		"Written by %s.\n",
		PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
		AUTHOR);
}

void show_all_usages(void)
{
	char *margin = mkspase(strlen(appname) + 8);

	printf("usage: %s [--version] [--help] [--db_pathname=<path>]\n"
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

void show_command_usage(const char *command)
{
	int optab['u' - 'C' + 1];

	optab['c' - 'C'] = optab['C' - 'C'] = 0;
	optab['r' - 'C'] = optab['R' - 'C'] = 1;
	optab['u' - 'C'] = optab['U' - 'C'] = 2;
	optab['d' - 'C'] = optab['D' - 'C'] = 3;

	const char *format;
	const char *(*fmthandler[4])(void) = {
		get_create_usage_format,
		get_read_usage_format,
		get_update_usage_format,
		get_delete_usage_format,
	};

	format = fmthandler[optab[*command - 'C']]();

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