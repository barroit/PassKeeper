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

void assign_field(sqlite3_stmt *stmt, int column, char **filed, int *flen);

void assign_by_large_value(int *dest, int tar);

int align_and_wrap_field(struct string_buffer *buf, const char *field, int field_crtlen, int field_maxlen, int wrap_threshold, const char *padstr);

void print_brief_field(struct string_buffer *buf, struct rcfield *data, int field_maxlen_map[3], int wrap_threshold, const char *padstr);

void print_verbose_field(struct string_buffer *buf, const struct rcfield *data, int *is_init);

#endif /* DATASTORE_H */