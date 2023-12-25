#include "datastore.h"
#include "utility.h"
#include "debug.h"

#define return_on_fail(rc, stmt)						\
	do									\
	{									\
		if (rc != SQLITE_OK)						\
		{								\
			return sqlite3_finalize(stmt);				\
		}								\
	}									\
	while (0)

int init_db_file(const char *filename, sqlite3 **db)
{
	if (is_rw_file(filename))
	{
		return FILENAME_EXISTS;
	}

	int rc;
	char *prt_name; // prt stands for parent

	prt_name = dirname(filename);

	if (prt_name == NULL)
	{
		return INVALID_FILENAME;
	}

	if (!is_rwx_dir(prt_name))
	{
		rc = MKDIR(prt_name);
	}

	free(prt_name);

	if (rc != EXEC_OK)
	{
		return MKDIR_FAILURE;
	}

	return sqlite3_open(filename, db);
}

int init_db_table(sqlite3 *db)
{
	const char *sql;

	sql = "CREATE TABLE account ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"sitename TEXT NOT NULL,"
		"siteurl TEXT,"
		"username TEXT,"
		"password TEXT,"
		"authtext TEXT,"
		"bakcode TEXT,"
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
		"(sitename, siteurl, username, password, authtext, bakcode, comment) "
		"VALUES"
		"(:sitename, :siteurl, :username, :password, :authtext, :bakcode, :comment);";

	return_on_fail(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL), stmt);

	return_on_fail(sqlite3_bind_text(stmt, 1, appopt->sitename, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 2, appopt->siteurl, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 3, appopt->username, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 4, appopt->password, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 5, appopt->authtext, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 6, appopt->bakcode, -1, SQLITE_STATIC), stmt);
	return_on_fail(sqlite3_bind_text(stmt, 7, appopt->comment, -1, SQLITE_STATIC), stmt);

	int rc;
	// If the most recent evaluation of statement S failed,
	// then sqlite3_finalize(S) returns the appropriate error code.
	// we can leave sqlite3_step without checking its rc
	rc = sqlite3_step(stmt);

	if (rc != SQLITE_DONE)
	{
		debug_message(puts(sqlite3_errmsg(db)));
	}
	
	return sqlite3_finalize(stmt);
}

int read_record(sqlite3 *db, const struct app_option *appopt)
{
	int rc;
	const char *sql;
	sqlite3_stmt *stmt;

	sql = "SELECT * FROM account WHERE sitename LIKE :sitename";
	rc = sqlite3_prepare_v3(db, sql, -1, SQLITE_PREPARE_PERSISTENT, &stmt, NULL);
	return_on_fail(rc, stmt);

	if (appopt->sitename == NULL)
	{
		rc = sqlite3_bind_text(stmt, 1, "%", 1, SQLITE_STATIC);
	}
	else
	{
		char *search_pattern = strapd(appopt->sitename, "%");
		rc = sqlite3_bind_text(stmt, 1, search_pattern, -1, SQLITE_TRANSIENT);
		free(search_pattern);
	}

	return_on_fail(rc, stmt);

	struct rcque *q;
	struct rcfield *data;
	int field_maxlen_map[3] = { 0, 0, 0 };

	rcqinit(&q);
	while (1)
	{
		rc = sqlite3_step(stmt);

		if (rc != SQLITE_ROW)
			break;

		rcfinit(&data);

		assign_field(stmt, 1, &data->sitename, &data->sitename_length);
		assign_by_large_value(&field_maxlen_map[0], data->sitename_length);

		assign_field(stmt, 3, &data->username, &data->username_length);
		assign_by_large_value(&field_maxlen_map[1], data->username_length);

		assign_field(stmt, 4, &data->password, &data->password_length);
		assign_by_large_value(&field_maxlen_map[2], data->password_length);

		if (appopt->is_verbose)
		{
			assign_field(stmt, 0, &data->id, NULL);
			assign_field(stmt, 2, &data->siteurl, NULL);
			assign_field(stmt, 5, &data->authtext, NULL);
			assign_field(stmt, 6, &data->bakcode, NULL);
			assign_field(stmt, 7, &data->comment, NULL);
			assign_field(stmt, 8, &data->sqltime, NULL);
			assign_field(stmt, 9, &data->modtime, NULL);
		}

		enrcque(q, data);
	}

	debug_execute({
		size_t sz = get_rcque_size(q);
		debug_message(printf("size of rcque is %lu\n", sz));
	});

	struct string_buffer *buf;
	char *padstr;

	sbinit(&buf);
	padstr = strpad(appopt->wrap_threshold);
	int is_init = 1;

	while ((data = dercque(q)) != NULL)
	{
		if (appopt->is_verbose)
		{
			print_verbose_field(buf, data, &is_init);
		}
		else
		{
			print_brief_field(buf, data, field_maxlen_map, appopt->wrap_threshold, padstr);
		}

		if (buf->size > 16 * 1000)
		{
			fputs(buf->data, stdout);
			sbclean(buf);
		}

		rcffree(data);
	}

	if (buf->size != 0)
	{
		fputs(buf->data, stdout);
	}

	free(padstr);
	sbfree(buf);
	rcqfree(q);

	debug_message(printf("strbuffer resize executed %d times\n", resize_execution_count));

	return sqlite3_finalize(stmt);
}

void assign_field(sqlite3_stmt *stmt, int column, char **field, int *flen)
{
	const char *tmpf = (const char *)sqlite3_column_text(stmt, column);
	int tmpflen = sqlite3_column_bytes(stmt, column) / sizeof(char);

	if (tmpf == NULL)
	{
		tmpf = "(null)";
		tmpflen = 6;
	}

	strndup(field, tmpf, tmpflen);

	if (flen != NULL)
	{
		*flen = tmpflen;
	}
}

void assign_by_large_value(int *dest, int tar)
{
	if (tar > *dest)
	{
		*dest = tar;
	}
}

int align_and_wrap_field(struct string_buffer *buf, const char *field, int field_crtlen, int field_maxlen, int wrap_threshold, const char *padstr)
{
	if (field == NULL)
	{
		sbprintf(buf, "%s", padstr);
		return 0;
	}

	char *field_fragment;
	int align_boundary, align_amount;
	int substr_length;

	align_boundary = MIN(field_maxlen, wrap_threshold);
	align_amount = align_boundary - field_crtlen;

	field_fragment = NULL;
	substr_length = 0;

	if (align_amount < 0)
	{
		substr_length = field_crtlen + align_amount;
		field_fragment = strsub(field, 0, substr_length);
	}

	const char *fmt_field, *fmt_padstr;
	int padstr_offset;

	padstr_offset = align_amount < 0 ? wrap_threshold : wrap_threshold - align_amount;

	fmt_field = field_fragment == NULL ? field : field_fragment;
	fmt_padstr = padstr + padstr_offset;

	sbnprintf(buf, align_boundary, "%s%s", fmt_field, fmt_padstr);

	if (align_amount < 0)
	{
		free(field_fragment);
	}

	return substr_length;
}

void print_brief_field(struct string_buffer *buf, struct rcfield *data, int field_maxlen_map[3], int wrap_threshold, const char *padstr)
{
	int substr_length[3];

	substr_length[0] = align_and_wrap_field(buf, data->sitename, data->sitename_length, field_maxlen_map[0], wrap_threshold, padstr);
	sbprint(buf, "  ");
	substr_length[1] = align_and_wrap_field(buf, data->username, data->username_length, field_maxlen_map[1], wrap_threshold, padstr);
	sbprint(buf, "  ");
	substr_length[2] = align_and_wrap_field(buf, data->password, data->password_length, field_maxlen_map[2], wrap_threshold, padstr);
	sbprint(buf, "\n");

	if (substr_length[0] + substr_length[1] + substr_length[2] == 0)
	{
		return;
	}

	char *rawstr_map[3] = { data->sitename, data->username, data->password };
	int rawlen_map[3] = { data->sitename_length, data->username_length, data->password_length };

	data->sitename = substr_length[0] == 0 ? NULL : data->sitename + substr_length[0];
	data->username = substr_length[1] == 0 ? NULL : data->username + substr_length[1];
	data->password = substr_length[2] == 0 ? NULL : data->password + substr_length[2];
	data->sitename_length = substr_length[0] == 0 ? 0 : data->sitename_length - substr_length[0];
	data->username_length = substr_length[1] == 0 ? 0 : data->username_length - substr_length[1];
	data->password_length = substr_length[2] == 0 ? 0 : data->password_length - substr_length[2];

	print_brief_field(buf, data, field_maxlen_map, wrap_threshold, padstr);

	data->sitename = rawstr_map[0];
	data->username = rawstr_map[1];
	data->password = rawstr_map[2];
	data->sitename_length = rawlen_map[0];
	data->username_length = rawlen_map[1];
	data->password_length = rawlen_map[2];
}

void print_verbose_field(struct string_buffer *buf, const struct rcfield *data, int *is_init)
{
	sbprintf(buf,
		"%s"
		"id -> %s\n"
		"sitename -> %s\n"
		"siteurl -> %s\n"
		"username -> %s\n"
		"password -> %s\n"
		"authtext -> %s\n"
		"bakcode -> %s\n"
		"comment -> %s\n"
		"sqltime -> %s\n"
		"modtime -> %s\n",
		*is_init ? (*is_init = false, "") : "\n",
		data->id,
		data->sitename,
		PRINTABLE_STRING(data->siteurl),
		PRINTABLE_STRING(data->username),
		PRINTABLE_STRING(data->password),
		PRINTABLE_STRING(data->authtext),
		PRINTABLE_STRING(data->bakcode),
		PRINTABLE_STRING(data->comment),
		PRINTABLE_STRING(data->sqltime),
		PRINTABLE_STRING(data->modtime));
}