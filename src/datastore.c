#include "datastore.h"
#include "utility.h"
#include "debug.h"

int init_db_file(const char *filename, sqlite3 **db)
{
	if (is_rw_file(filename))
		return FILENAME_EXISTS;

	int rc;
	char *dirname;

	rc = get_parent_dir(&dirname, filename);

	if (rc != EXEC_OK)
		return INVALID_FILENAME;

	if (!is_rwx_dir(dirname))
		rc = MKDIR(dirname);

	free(dirname);

	if (rc != EXEC_OK)
		return MKDIR_FAILURE;

	return sqlite3_open(filename, db);
}

int init_db_table(sqlite3 *db)
{
	const char *sql;

	sql = "CREATE TABLE account ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"site_name TEXT NOT NULL,"
		"site_url TEXT,"
		"username TEXT,"
		"password TEXT,"
		"auth_text TEXT,"
		"recovery_code TEXT,"
		"comment TEXT,"
		"sqltime DATETIME DEFAULT (datetime('now', '+9 hours')),"
		"modtime DATETIME"
	");"
	"CREATE TRIGGER update_modtime "
	"AFTER UPDATE ON account "
	"FOR EACH ROW "
	"BEGIN "
		"UPDATE account SET modtime = datetime('now', '+9 hours') WHERE id = old.id;"
	"END;";

	return sqlite3_exec(db, sql, NULL, NULL, NULL);
}

int create_record(sqlite3 *db, const struct app_option *appopt)
{
	const char *sql;
	sqlite3_stmt *stmt;

	sql = "INSERT INTO account "
		"(site_name, site_url, username, password, auth_text, recovery_code, comment) "
		"VALUES"
		"(:site_name, :site_url, :username, :password, :auth_text, :recovery_code, :comment);";

	return_on_fail(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL), stmt);

	return_on_fail(sqlite3_bind_text(stmt, 1, appopt->site_name, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 2, appopt->site_url, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 3, appopt->username, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 4, appopt->password, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 5, appopt->auth_text, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 6, appopt->recovery_code, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 7, appopt->comment, -1, SQLITE_STATIC), stmt);

	int rc;
	// If the most recent evaluation of statement S failed,
	// then sqlite3_finalize(S) returns the appropriate error code.
	// we can leave sqlite3_step without checking its rc
	rc = sqlite3_step(stmt);

	if (rc != SQLITE_DONE)
		debug_only(puts(sqlite3_errmsg(db)));
	
	return sqlite3_finalize(stmt);
}

int read_record(sqlite3 *db, const struct app_option *appopt)
{
	int rc;
	sqlite3_stmt *stmt;
	char *search_pattern;
	const char *sql;

	sql = "SELECT * FROM account WHERE site_name LIKE :site_name";

	rc = sqlite3_prepare_v3(db, sql, -1, SQLITE_PREPARE_PERSISTENT, &stmt, NULL);
	return_on_fail(rc, stmt);
	
	strndup(&search_pattern, appopt->site_name, -1);
	strapd(&search_pattern, "%");

	rc = sqlite3_bind_text(stmt, 1, search_pattern, -1, appopt->site_name == NULL ? SQLITE_TRANSIENT : SQLITE_STATIC);
	free(search_pattern);
	return_on_fail(rc, stmt);

	struct queue *q;
	struct field *data;

	q = make_queue();

	while (1)
	{
		rc = sqlite3_step(stmt);

		if (rc != SQLITE_ROW)
			break;

		data = make_field();

		process_field(stmt, 1, &data->site_name);
		process_field(stmt, 3, &data->username);
		process_field(stmt, 4, &data->password);

		if (appopt->is_verbose)
		{
			process_field(stmt, 0, &data->id);
			process_field(stmt, 2, &data->site_url);
			process_field(stmt, 5, &data->auth_text);
			process_field(stmt, 6, &data->recovery_code);
			process_field(stmt, 7, &data->comment);
			process_field(stmt, 8, &data->sqltime);
			process_field(stmt, 9, &data->modtime);
		}

		enqueue(q, data);
	}

	debug_only(print_queue_size(q));

	while ((data = dequeue(q)) != NULL)
	{
		if (appopt->is_verbose)
			print_verbose_field(data);
		else
			print_brief_field(data, appopt->threshold);
		free_field(data);
	}

	free_queue(q);

	return sqlite3_finalize(stmt);
}

void process_field(sqlite3_stmt *stmt, int column, char **filed)
{
	const char *tmpf = (const char *)sqlite3_column_text(stmt, column);
	int tmpflen = sqlite3_column_bytes(stmt, column) / sizeof(char);

	strndup(filed, tmpf, tmpflen);
}

void print_brief_field(const struct field *data, int wrap_threshold)
{
	char *site_name = data->site_name == NULL ? 
	strlen()
	data->site_name
	// maxlen > wrap_threshold, wrap required
	// maxlen <= wrap_threshold, align withe maxlen
	puts(data->site_name);
}