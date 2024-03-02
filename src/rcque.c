#include "rcque.h"
#include "misc.h"
#include "debug.h"
#include "rescode.h"

#ifdef PK_IS_DEBUG
size_t rcque_size = 0;
#endif

recordqueue *rcqmake(void)
{
	recordqueue *q;
	if ((q = malloc(sizeof(recordqueue))) == NULL)
	{
		return NULL;
	}

	q->front = NULL;
	q->back = NULL;

	return q;
}

recordfield *rcfmake(void)
{
	recordfield *f;
	if ((f = malloc(sizeof(recordfield))) == NULL)
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

recordfield *enrcque(recordqueue *q, recordfield *data)
{
	recordnode *n;
	if ((n = malloc(sizeof(recordnode))) == NULL)
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

	debug_execute(rcque_size++);

	return data;
}

recordfield *dercque(recordqueue *q)
{
	if (q->front == NULL)
	{
		return NULL;
	}

	recordnode *tmp = q->front;
	recordfield *data = tmp->data;

	if ((q->front = q->front->next) == NULL)
	{
		q->back = NULL;
	}

	free(tmp);

	debug_execute(rcque_size--);

	return data;
}

void rcffree(recordfield *data)
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

void rcqfree(recordqueue *q)
{
	if (q == NULL)
	{
		return;
	}

	recordfield *data;
	while ((data = dercque(q)) != NULL)
	{
		rcffree(data);
	}

	free(q);
}