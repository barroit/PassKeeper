#ifndef DATASTORE_H
#define DATASTORE_H

#include "cmdparser.h"
#include "rcque.h"
#include "strbuffer.h"
#include <sqlite3.h>

#define FILENAME_EXISTS		37
#define INVALID_FILENAME	38
#define MKDIR_FAILURE		39

int init_db_file(const char *filename, sqlite3 **db);

int init_db_table(sqlite3 *db);

int create_record(sqlite3 *db, const struct app_option *appopt);

int read_record(sqlite3 *db, const struct app_option *appopt);

void process_field(sqlite3_stmt *stmt, int column, char **filed, int *flen);

void print_brief_field(const struct rcfield *data, int wrap_threshold);

void print_verbose_field(struct string_buffer *buf, const struct rcfield *data, int *is_init);

#endif /* DATASTORE_H */