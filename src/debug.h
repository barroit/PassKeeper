#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG

#include <stdio.h>

#define debug_only(stmt)		\
	do				\
	{				\
		puts("[debug]");	\
		(stmt);			\
		puts("[gubed]\n");	\
	}				\
	while (0)


struct app_option;
struct queue;

void print_appopt(const struct app_option *appopt);

void print_queue_size(const struct queue *q);

#else /* DEBUG */

#define debug_only(stmt) ((void)0)

#endif /* DEBUG */

#endif /* DEBUG_H */