#include "io.h"
#include "strbuffer.h"
#include "utility.h"
#include "os.h"
#include "rescode.h"

#include <stdlib.h>
#include <stdio.h>

int prepare_file_folder(const char *pathname)
{
	char *dirname;
	if ((dirname = prefix(pathname)) == NULL)
	{
		return PK_INVALID_PATHNAME;
	}

	int rc = PK_SUCCESS;
	if (!is_rwx_dir(dirname))
	{
		rc = dirmake(dirname);
	}

	free(dirname);

	return rc == PK_SUCCESS ? PK_SUCCESS : PK_MKDIR_FAILURE;
}

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

#ifdef PK_USE_FHS
#include <sys/stat.h>
#endif

int dirmake(const char *pathname)
{
#ifdef PK_USE_FHS
	return mkdir(pathname, 0755);
#else
	return mkdir(pathname);
#endif
}