#ifndef DATASTORE_H
#define DATASTORE_H

struct app_option;
struct sqlite3;
typedef struct sqlite3 sqlite3;

int init_scheme(sqlite3 *db);

int create_record(sqlite3 *db, const struct app_option *appopt);

int read_record(sqlite3 *db, const struct app_option *appopt);

int update_record(sqlite3 *db, const struct app_option *appopt);

int delete_record(sqlite3 *db, const struct app_option *appopt);

#endif /* DATASTORE_H */