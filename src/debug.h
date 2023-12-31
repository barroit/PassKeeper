#ifndef DEBUG_H
#define DEBUG_H

#ifdef PK_IS_DEBUG

#include <stdio.h>

#define debug_log(...)			\
	do				\
	{				\
		puts("\n[debug]");	\
		printf(__VA_ARGS__);	\
		puts("[gubed]\n");	\
	}				\
	while (0)

#define debug_execute(stmt)		\
	do				\
	{				\
		(stmt);			\
	}				\
	while (0)

#else /* PK_IS_DEBUG */

#define debug_log(...) ((void)0)

#define debug_execute(stmt) ((void)0)

#endif /* PK_IS_DEBUG */

#endif /* DEBUG_H */