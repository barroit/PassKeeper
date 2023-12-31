#ifndef RCQUE_H
#define RCQUE_H

#include <stddef.h>

#ifdef PK_IS_DEBUG

extern size_t record_queue_size;

#endif /* PK_IS_DEBUG */

typedef struct
{
	char *id;
	char *sitename;
	char *siteurl;
	char *username;
	char *password;
	char *authtext;
	char *bakcode;
	char *comment;
	char *sqltime;
	char *modtime;

	int sitename_length;
	int username_length;
	int password_length;
} record_field;

typedef struct node
{
	record_field *data;
	struct node *next;
} record_node;

typedef struct
{
	record_node *front;
	record_node *back;
} record_queue;

record_queue *rcqmake(void);

record_field *rcfmake(void);

record_field *enrcque(record_queue *q, record_field *data);

record_field *dercque(record_queue *q);

void rcffree(record_field *data);

void rcqfree(record_queue *q);

#endif /* RCQUE_H */