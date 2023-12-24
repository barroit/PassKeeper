#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>

#define SUPPRESS_OUTPUT(fn) \
	do \
	{ \
		fflush(stdout); \
		FILE* rawout = stdout; \
		stdout = freopen("/dev/null", "w", stdout); \
		if (stdout) { \
			(fn); \
			fflush(stdout); \
			fclose(stdout); \
			stdout = rawout; \
		} \
	} \
	while (0)

#define MOCK_FILE(filename, fn) \
	do \
	{ \
		FILE *fptr; \
		fptr = fopen(filename, "w"); \
		fclose(fptr); \
		(fn); \
		remove(filename); \
	} \
	while (0)

#endif /* HELPER_H */