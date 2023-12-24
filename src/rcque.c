#include "rcque.h"
#include "utility.h"

int rcqinit(struct rcque **q)
{
	if ((*q = malloc(sizeof(struct rcque))) == NULL)
		return MALLOC_FAILURE;

	(*q)->front = NULL;
	(*q)->back = NULL;

	return EXEC_OK;
}

int rcfinit(struct rcfield **f)
{
	if ((*f = malloc(sizeof(struct rcfield))) == NULL)
		return MALLOC_FAILURE;

	(*f)->id = NULL;
	(*f)->site_name = NULL;
	(*f)->site_url = NULL;
	(*f)->username = NULL;
	(*f)->password = NULL;
	(*f)->auth_text = NULL;
	(*f)->recovery_code = NULL;
	(*f)->comment = NULL;
	(*f)->sqltime = NULL;
	(*f)->modtime = NULL;

	return EXEC_OK;
}

int enrcque(struct rcque *q, struct rcfield *data)
{
	struct rcnode *n;
	if ((n = malloc(sizeof(struct rcnode))) == NULL)
		return MALLOC_FAILURE;

	n->data = data;
	n->next = NULL;

	// <-----|
	//     front ----------> back
	//                        |-----<
	if (q->back == NULL)
		q->front = n;
	else
		q->back->next = n;
	q->back = n;

	return EXEC_OK;
}

struct rcfield *dercque(struct rcque *q)
{
	if (q->front == NULL)
		return NULL;

	struct rcnode *tmp = q->front;
	struct rcfield *data = tmp->data;

	if ((q->front = q->front->next) == NULL)
		q->back = NULL;

	free(tmp);
	return data;
}

void rcffree(struct rcfield *data)
{
	if (data == NULL)
		return;

	free(data->id);
	free(data->site_name);
	free(data->site_url);
	free(data->username);
	free(data->password);
	free(data->auth_text);
	free(data->recovery_code);
	free(data->comment);
	free(data->sqltime);
	free(data->modtime);
	free(data);
}

void rcqfree(struct rcque *q)
{
	if (q == NULL)
		return;

	struct rcfield *data;
	while ((data = dercque(q)) != NULL)
		rcffree(data);

	free(q);
}