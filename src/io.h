#ifndef IO_H
#define IO_H

#include <stddef.h>

int prepare_file_folder(const char *pathname);

char *read_file_content(const char *pathname, size_t *size);

int dirmake(const char *pathname);

#endif /* IO_H */