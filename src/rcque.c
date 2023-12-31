#include "rcque.h"
#include "utility.h"
#include "debug.h"
#include "rescode.h"

#include <stdlib.h>

#ifdef PK_IS_DEBUG

size_t record_queue_size = 0;

#endif /* PK_IS_DEBUG */

record_queue *rcqmake(void)
{
	record_queue *q;
	if ((q = malloc(sizeof(record_queue))) == NULL)
	{
		return NULL;
	}

	q->front = NULL;
	q->back = NULL;

	return q;
}

record_field *rcfmake(void)
{
	record_field *f;
	if ((f = malloc(sizeof(record_field))) == NULL)
	{
		return NULL;
	}

	f->id = NULL;
	f->sitename = NULL;
	f->siteurl = NULL;
	f->username = NULL;
	f->password = NULL;
	f->authtext = NULL;
	f->bakcode = NULL;
	f->comment = NULL;
	f->sqltime = NULL;
	f->modtime = NULL;

	f->sitename_length = 0;
	f->username_length = 0;
	f->password_length = 0;

	return f;
}

record_field *enrcque(record_queue *q, record_field *data)
{
	record_node *n;
	if ((n = malloc(sizeof(record_node))) == NULL)
	{
		return NULL;
	}

	n->data = data;
	n->next = NULL;

	// <-----|
	//     front ----------> back
	//                        |-----<
	if (q->back == NULL)
	{
		q->front = n;
	}
	else
	{
		q->back->next = n;
	}
	q->back = n;

	debug_execute(record_queue_size++);

	return data;
}

record_field *dercque(record_queue *q)
{
	if (q->front == NULL)
	{
		return NULL;
	}

	record_node *tmp = q->front;
	record_field *data = tmp->data;

	if ((q->front = q->front->next) == NULL)
	{
		q->back = NULL;
	}

	free(tmp);

	debug_execute(record_queue_size--);

	return data;
}

void rcffree(record_field *data)
{
	if (data == NULL)
		return;

	free(data->id);
	free(data->sitename);
	free(data->siteurl);
	free(data->username);
	free(data->password);
	free(data->authtext);
	free(data->bakcode);
	free(data->comment);
	free(data->sqltime);
	free(data->modtime);
	free(data);
}

void rcqfree(record_queue *q)
{
	if (q == NULL)
	{
		return;
	}

	record_field *data;
	while ((data = dercque(q)) != NULL)
	{
		rcffree(data);
	}

	free(q);
}