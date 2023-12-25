#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG

#include <stdio.h>
#include <stddef.h>

#define debug_message(stmt)		\
	do				\
	{				\
		puts("\n[debug]");	\
		(stmt);			\
		puts("[gubed]\n");	\
	}				\
	while (0)

#define debug_execute(stmt)		\
	do				\
	{				\
		(stmt);			\
	}				\
	while (0)

struct app_option;
struct rcque;

void print_appopt(const struct app_option *appopt);

size_t get_rcque_size(const struct rcque *q);

#else /* DEBUG */

#define debug_message(stmt) ((void)0)

#define debug_execute(stmt) ((void)0)

#endif /* DEBUG */

#endif /* DEBUG_H */