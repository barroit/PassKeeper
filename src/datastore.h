#ifndef DATASTORE_H
#define DATASTORE_H

#include "cmdparser.h"
#include "rcque.h"
#include "strbuf.h"

#include <stdbool.h>
#include <sqlite3.h>

typedef struct
{
	const char *key;
	const void *value;
} entry;

int init_db_file(sqlite3 **db, const char *dbpath, const char *errmsg[2]);

int encrypt_db(sqlite3 *db, const char *keypath, const char *errmsg[2]);

int decrypt_db(sqlite3 *db, const char *keypath, const char *errmsg[2]);

int read_db_key(const char *keypath, void **__key, size_t *__ksz);

int init_db_key(const char *keypath, void **__key, size_t *__ksz);

int init_db_table(sqlite3 *db);

bool is_db_decrypted(sqlite3 *db);

int create_record(sqlite3 *db, const app_option *appopt, char **message);

int read_record(sqlite3 *db, const app_option *appopt, char **message);

void assign_field(sqlite3_stmt *stmt, int column, char **filed, int *flen);

void assign_by_large_value(int *dest, int tar);

int align_and_wrap_field(stringbuffer *buf, const char *field, int field_crtlen, int field_maxlen, int wrap_threshold, const char *padstr);

void print_brief_field(stringbuffer *buf, recordfield *data, int field_maxlen_map[3], int wrap_threshold, const char *padstr);

void print_verbose_field(stringbuffer *buf, const recordfield *data, int *is_init);

int update_record(sqlite3 *db, const app_option *appopt, char **message);

int delete_record(sqlite3 *db, const app_option *appopt, char **message);

#endif /* DATASTORE_H */