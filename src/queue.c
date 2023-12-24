#include "queue.h"
#include "utility.h"

struct queue *make_queue(void)
{
	struct queue *q;
	if ((q = (struct queue *)malloc(sizeof(struct queue))) == NULL)
		return NULL;

	q->front = NULL;
	q->back = NULL;

	return q;
}

struct field *make_field(void)
{
	struct field *f;
	if ((f = (struct field *)malloc(sizeof(struct field))) == NULL)
		return NULL;

	f->id = NULL;
	f->site_name = NULL;
	f->site_url = NULL;
	f->username = NULL;
	f->password = NULL;
	f->auth_text = NULL;
	f->recovery_code = NULL;
	f->comment = NULL;
	f->sqltime = NULL;
	f->modtime = NULL;

	return f;
}

struct field *enqueue(struct queue *q, struct field *data)
{
	struct qnode *node;
	if ((node = (struct qnode *)malloc(sizeof(struct qnode))) == NULL)
		return NULL;

	node->data = data;
	node->next = NULL;

	// <-----|
	//     front ----------> back
	//                        |-----<
	if (q->back == NULL)
		q->front = node;
	else
		q->back->next = node;
	q->back = node;

	return data;
}

struct field *dequeue(struct queue *q)
{
	if (q->front == NULL)
		return NULL;

	struct qnode *tmp = q->front;
	struct field *data = tmp->data;

	if ((q->front = q->front->next) == NULL)
		q->back = NULL;

	free(tmp);
	return data;
}

void free_field(struct field *data)
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

void free_queue(struct queue *q)
{
	if (q == NULL)
		return;

	struct field *data;
	while ((data = dequeue(q)) != NULL)
		free_field(data);

	free(q);
}