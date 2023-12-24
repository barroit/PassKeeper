#include "debug.h"
#include "utility.h"
#include "cmdparser.h"
#include "queue.h"

void print_appopt(const struct app_option *appopt)
{
	printf("db_filename: %s\n"
		"command: %s\n"
		"is_help: %s\n"
		"is_version: %s\n"
		"is_verbose: %s\n"
		"is_db_init: %s\n"
		"record_id: %d\n"
		"site_name: %s\n"
		"site_url: %s\n"
		"username: %s\n"
		"password: %s\n"
		"auth_text: %s\n"
		"recovery_code: %s\n"
		"comment: %s\n",
		PRINTABLE_STRING(appopt->db_filename),
		PRINTABLE_STRING(appopt->command),
		READABLE_BOOLEAN(appopt->is_help),
		READABLE_BOOLEAN(appopt->is_version),
		READABLE_BOOLEAN(appopt->is_verbose),
		READABLE_BOOLEAN(appopt->is_db_init),
		appopt->record_id,
		PRINTABLE_STRING(appopt->site_name),
		PRINTABLE_STRING(appopt->site_url),
		PRINTABLE_STRING(appopt->username),
		PRINTABLE_STRING(appopt->password),
		PRINTABLE_STRING(appopt->auth_text),
		PRINTABLE_STRING(appopt->recovery_code),
		PRINTABLE_STRING(appopt->comment));
}

void print_queue_size(const struct queue *q)
{
	size_t size;
	struct qnode *crt;

	size = 0;
	crt = q->front;
	while (crt != NULL)
	{
		size++;
		crt = crt->next;
	}

	printf("size of q: %lu\n", size);
}