#include "debug.h"
#include "utility.h"
#include "cmdparser.h"
#include "rcque.h"

void print_appopt(const struct app_option *appopt)
{
	printf("db_filename: %s\n"
		"command: %s\n"
		"is_help: %s\n"
		"is_version: %s\n"
		"is_verbose: %s\n"
		"is_db_init: %s\n"
		"record_id: %d\n"
		"sitename: %s\n"
		"siteurl: %s\n"
		"username: %s\n"
		"password: %s\n"
		"authtext: %s\n"
		"bakcode: %s\n"
		"comment: %s\n",
		PRINTABLE_STRING(appopt->db_filename),
		PRINTABLE_STRING(appopt->command),
		READABLE_BOOLEAN(appopt->is_help),
		READABLE_BOOLEAN(appopt->is_version),
		READABLE_BOOLEAN(appopt->is_verbose),
		READABLE_BOOLEAN(appopt->is_db_init),
		appopt->record_id,
		PRINTABLE_STRING(appopt->sitename),
		PRINTABLE_STRING(appopt->siteurl),
		PRINTABLE_STRING(appopt->username),
		PRINTABLE_STRING(appopt->password),
		PRINTABLE_STRING(appopt->authtext),
		PRINTABLE_STRING(appopt->bakcode),
		PRINTABLE_STRING(appopt->comment));
}

size_t get_rcque_size(const struct rcque *q)
{
	size_t size;
	struct rcnode *crt;

	size = 0;
	crt = q->front;
	while (crt != NULL)
	{
		size++;
		crt = crt->next;
	}

	return size;
}