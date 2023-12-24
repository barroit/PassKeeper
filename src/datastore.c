#include "datastore.h"
#include "utility.h"
#include "debug.h"

#define return_on_fail(rc, stmt)						\
	do									\
	{									\
		if (rc != SQLITE_OK)						\
			return sqlite3_finalize(stmt);				\
	}									\
	while (0)

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

	struct rcque *q;
	struct rcfield *data;
	int maxlen_map[3] = { 0, 0, 0 };

	rcqinit(&q);
	while (1)
	{
		rc = sqlite3_step(stmt);

		if (rc != SQLITE_ROW)
			break;

		rcfinit(&data);

		process_field(stmt, 1, &data->site_name, &data->sitename_length);
		if (data->sitename_length > maxlen_map[0])
			maxlen_map[0] = data->sitename_length;
		process_field(stmt, 3, &data->username, &data->username_length);
		if (data->username_length > maxlen_map[1])
			maxlen_map[1] = data->username_length;
		process_field(stmt, 4, &data->password, &data->password_length);
		if (data->password_length > maxlen_map[2])
			maxlen_map[2] = data->password_length;

		if (appopt->is_verbose)
		{
			process_field(stmt, 0, &data->id, NULL);
			process_field(stmt, 2, &data->site_url, NULL);
			process_field(stmt, 5, &data->auth_text, NULL);
			process_field(stmt, 6, &data->recovery_code, NULL);
			process_field(stmt, 7, &data->comment, NULL);
			process_field(stmt, 8, &data->sqltime, NULL);
			process_field(stmt, 9, &data->modtime, NULL);
		}

		enrcque(q, data);
	}

	debug_only(print_queue_size(q));

	struct string_buffer *buf;
	char *space;

	int threshold = appopt->wrap_threshold - 2;
	int space_size = threshold - 1; // should contain at least one char
	get_space(&space, space_size);

	int is_init = 1;
	sbinit(&buf);
	while ((data = dercque(q)) != NULL)
	{
		// if (appopt->is_verbose)
		// 	print_verbose_field(buf, data, &is_init);

		// if (buf->size > 16 * 1000)
		// {
		// 	fputs(buf->data, stdout);
		// 	sbclean(buf);
		// }

		// maxlen < wrap_threshold - 2, use maxlen as threshold
		// else, use wrap_threshold

		// | abc  |

		int align;
		if (maxlen_map[0] <= threshold)
		{
			align = maxlen_map[0] - data->sitename_length;
			sbprintf(buf, " %s%s ", data->site_name, space + (space_size - align));
		}

		rcffree(data);
	}

	if (buf->size)
		fputs(buf->data, stdout);

	free(space);

	sbfree(buf);
	rcqfree(q);

	return sqlite3_finalize(stmt);
}

void process_field(sqlite3_stmt *stmt, int column, char **filed, int *flen)
{
	const char *tmpf = (const char *)sqlite3_column_text(stmt, column);
	int tmpflen = sqlite3_column_bytes(stmt, column) / sizeof(char);

	strndup(filed, tmpf, tmpflen);

	if (flen != NULL)
		*flen = tmpflen;
}

void print_verbose_field(struct string_buffer *buf, const struct rcfield *data, int *is_init)
{
	sbprintf(buf,
		"%s"
		"id -> %s\n"
		"site_name -> %s\n"
		"site_url -> %s\n"
		"username -> %s\n"
		"password -> %s\n"
		"auth_text -> %s\n"
		"recovery_code -> %s\n"
		"comment -> %s\n"
		"sqltime -> %s\n"
		"modtime -> %s\n",
		*is_init ? (*is_init = false, "") : "\n",
		data->id,
		data->site_name,
		PRINTABLE_STRING(data->site_url),
		PRINTABLE_STRING(data->username),
		PRINTABLE_STRING(data->password),
		PRINTABLE_STRING(data->auth_text),
		PRINTABLE_STRING(data->recovery_code),
		PRINTABLE_STRING(data->comment),
		PRINTABLE_STRING(data->sqltime),
		PRINTABLE_STRING(data->modtime));
}