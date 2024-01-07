#include "fileio.h"
#include "strbuffer.h"
#include "utility.h"
#include "rescode.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

char *read_file_content(const char *pathname, size_t *size)
{
	FILE *file;
	if ((file = fopen(pathname, "r")) == NULL)
	{
		return NULL;
	}

	string_buffer *buf;
	char c, *res;

	buf = sbmake(125);
	while ((c = fgetc(file)) != EOF)
	{
		sbputc(buf, c);
	}

	res = strsub(buf->data, 0, 0);

	if (size != NULL)
	{
		*size = buf->size;
	}

	sbfree(buf);
	fclose(file);

	return res;
}

int prepare_file_folder(const char *pathname)
{
	char *dirname;
	if ((dirname = prefix(pathname)) == NULL)
	{
		return PK_INVALID_PATHNAME;
	}

	int rc;

	rc = mkdir_p(dirname);
	free(dirname);

	return rc;
}

#ifdef __linux
#include <sys/stat.h>
#else
#include <unistd.h>
#endif

/* no error if existing */
int mkdir_p(const char *pathname)
{
	if (exists(pathname))
	{
		return PK_SUCCESS;
	}

	int rc;

#ifdef __linux
	rc = mkdir(pathname, 0755);
#else
	rc = mkdir(pathname);
#endif
	return rc == 0 ? PK_SUCCESS : PK_MKDIR_FAILURE;
}