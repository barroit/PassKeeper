#include "debug.h"

#ifdef DEBUG

#include "parse_command.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>

void print_appopt(const struct app_option *appopt)
{
	printf("---------------- START PRINT ----------------\n"
               "db_file: %s\n"
               "command: %s\n"
               "is_help: %s\n"
               "is_version: %s\n"
               "is_verbose: %s\n"
               "record_id: %d\n"
               "site_name: %s\n"
               "site_url: %s\n"
               "username: %s\n"
               "password: %s\n"
               "auth_text: %s\n"
               "recovery_code: %s\n"
               "comment: %s\n"
               "---------------- END PRINT ----------------\n",
               PRINTABLE_STRING(appopt->db_file),
               PRINTABLE_STRING(appopt->command),
               READABLE_BOOLEAN(appopt->is_help),
               READABLE_BOOLEAN(appopt->is_version),
               READABLE_BOOLEAN(appopt->is_verbose),
               appopt->record_id,
               PRINTABLE_STRING(appopt->site_name),
               PRINTABLE_STRING(appopt->site_url),
               PRINTABLE_STRING(appopt->username),
               PRINTABLE_STRING(appopt->password),
               PRINTABLE_STRING(appopt->auth_text),
               PRINTABLE_STRING(appopt->recovery_code),
               PRINTABLE_STRING(appopt->comment));
}

#endif /* DEBUG */