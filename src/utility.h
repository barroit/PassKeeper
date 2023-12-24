#ifndef UTILITY_H
#define UTILITY_H

#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

// program exit code
#define EXIT_PROMPT		1

// function result code
#define EXEC_OK			0
#define INVLIAD_ARGUMENT	60
#define MALLOC_FAILURE		61
#define MISSING_SEPERATOR	62

#define fatal(code, format, appname, ...)					\
	do									\
	{									\
		fprintf(stderr, "%s: " format "\n", appname, ##__VA_ARGS__);	\
		exit(code);							\
	}									\
	while (0)

#define REPORT_ERROR(format, appname, ...) fprintf(stderr, "%s: " format "\n", appname, ##__VA_ARGS__);

#define PRINTABLE_STRING(str) ((str) == NULL ? "(null)" : (str)[0] == '\0' ? "(empty)" : (str))

#define READABLE_BOOLEAN(v) ((v) ? "true" : "false")

#ifdef _WIN64
#define MKDIR(path) mkdir(path)
#else
#define MKDIR(path) mkdir(path, 0775)
#endif

/// check if the `string` is a positive integer
bool is_positive_integer(const char *str);

/// test the `dirname` with permission drwx...
bool is_rwx_dir(const char *dirname);

/// test the `filename` with permission -rw-...
bool is_rw_file(const char *filename);

/// check if `string` is empty
/// if the `string` is null pointer, then falsy value returned
bool is_empty_string(const char *string);

int get_space(char **space, int length);

int get_parent_dir(char **dirname, const char *filename);

int strndup(char **dest, const char *src, size_t n);

int strapd(char **dest, const char *src);

#endif /* UTILITY_H */