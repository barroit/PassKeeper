#ifndef DATASTORE_H
#define DATASTORE_H

#include "cmdparser.h"
#include "rcque.h"
#include "strbuffer.h"

#include <stddef.h>
#include <stdbool.h>
#include <sqlite3.h>

int init_database(const char *db_pathname, const char *db_key_pathname);

int encrypt_database(const char *db_key_pathname);

bool is_db_decrypted(sqlite3 *db);

int create_record(sqlite3 *db, const app_option *appopt);

int read_record(sqlite3 *db, const app_option *appopt);

void assign_field(sqlite3_stmt *stmt, int column, char **filed, int *flen);

void assign_by_large_value(int *dest, int tar);

int align_and_wrap_field(string_buffer *buf, const char *field, int field_crtlen, int field_maxlen, int wrap_threshold, const char *padstr);

void print_brief_field(string_buffer *buf, record_field *data, int field_maxlen_map[3], int wrap_threshold, const char *padstr);

void print_verbose_field(string_buffer *buf, const record_field *data, int *is_init);

#endif /* DATASTORE_H */