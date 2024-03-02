#ifndef RCQUE_H
#define RCQUE_H

#ifdef PK_IS_DEBUG
extern size_t rcque_size;
#endif

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

} recordfield;

typedef struct node
{
	recordfield *data;
	struct node *next;

} recordnode;

typedef struct
{
	recordnode *front;
	recordnode *back;

} recordqueue;

recordqueue *rcqmake(void);

recordfield *rcfmake(void);

recordfield *enrcque(recordqueue *q, recordfield *data);

recordfield *dercque(recordqueue *q);

void rcffree(recordfield *data);

void rcqfree(recordqueue *q);

#endif /* RCQUE_H */