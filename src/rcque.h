#ifndef RCQUE_H
#define RCQUE_H

struct rcfield
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
};

struct rcnode
{
	struct rcfield *data;
	struct rcnode *next;
};

struct rcque
{
	struct rcnode *front;
	struct rcnode *back;
};

int rcqinit(struct rcque **q);

int rcfinit(struct rcfield **f);

int enrcque(struct rcque *q, struct rcfield *data);

struct rcfield *dercque(struct rcque *q);

void rcffree(struct rcfield *data);

void rcqfree(struct rcque *q);

#endif /* RCQUE_H */