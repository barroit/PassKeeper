#ifndef PKIO_H
#define PKIO_H

#include <stddef.h>

int prepare_file_folder(const char *pathname);

char *read_file_content(const char *pathname, size_t *size);

int mkdir_p(const char *pathname);

#endif /* PKIO_H */