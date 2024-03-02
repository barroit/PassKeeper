#include "datastore.h"
#include "misc.h"
#include "strutil.h"
#include "debug.h"
#include "rescode.h"
#include "fileio.h"
#include "encrypt.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>

#define BINKEY_SIZE 32

int init_db_file(sqlite3 **db, const char *dbpath, const char *errmsg[2])
{
	int rc;

	if (exists(dbpath))
	{
		errmsg[ERRMSG_IV] = dbpath;
		return PK_FILE_EXISTS;
	}

	if ((rc = prepare_folder(dbpath)) != PK_SUCCESS)
	{
		errmsg[ERRMSG_IV] = dbpath;
		return rc;
	}

	return sqlite3_open(dbpath, db);
}

int encrypt_db(sqlite3 *db, const char *keypath, const char *errmsg[2])
{
	int rc;
	void *key;
	size_t ksz;

	if (exists(keypath))
	{
		rc = read_db_key(keypath, &key, &ksz);
	}
	else
	{
		rc = init_db_key(keypath, &key, &ksz);
	}

	if (rc != PK_SUCCESS && rc != PK_KEYGEN_FAILURE)
	{
		errmsg[ERRMSG_IV] = keypath;
	}

	if (rc != PK_SUCCESS)
	{
		return rc;
	}

	sqlite3_key(db, key, ksz);

	free(key);

	return PK_SUCCESS;
}

int decrypt_db(sqlite3 *db, const char *keypath, const char *errmsg[2])
{
	int rc;
	void *key;
	size_t ksz;

	if (!exists(keypath))
	{
		errmsg[ERRMSG_IV] = keypath;
		return PK_MISSING_FILE;
	}
	
	if ((rc = read_db_key(keypath, &key, &ksz)) != PK_SUCCESS)
	{
		errmsg[ERRMSG_IV] = keypath;
		return rc;
	}

	sqlite3_key(db, key, ksz);

	free(key);

	return PK_SUCCESS;
}

int read_db_key(const char *keypath, void **__key, size_t *__ksz)
{
	char *keystr;
	size_t ksz;
	if (!is_rw_file(keypath) || (keystr = read_content(keypath, &ksz)) == NULL)
	{
		return PK_PERMISSION_DENIED;
	}

	void *key;
	if ((ksz == BINKEY_SIZE * 2 + 2) && keystr[0] == '0' && keystr[1] == 'x')
	{
		key = hex_to_bin(keystr + 2, &ksz); /* replace ksz with the binary's */
	}
	else
	{
		key = substr(keystr, 0, ksz);
	}

	free(keystr);

	if (key == NULL) /* hex_to_bin failure */
	{
		return PK_INVALID_KEY;
	}

	if (__ksz != NULL)
	{
		*__ksz = ksz;
	}

	assert(__key != NULL);
	*__key = key;

	return PK_SUCCESS;
}

int init_db_key(const char *keypath, void **__key, size_t *__ksz)
{
	int rc;
	if ((rc = prepare_folder(keypath)) != PK_SUCCESS)
	{
		return rc;
	}

	FILE *file;
	if ((file = fopen(keypath, "w")) == NULL)
	{
		return PK_PERMISSION_DENIED;
	}

	void *binkey;
	char *hexstr;
	if ((binkey = get_binary_key(BINKEY_SIZE)) == NULL)
	{
		fclose(file);
		return PK_KEYGEN_FAILURE;
	}

	fprintf(file, "0x%s", (hexstr = bin_to_hex(binkey, BINKEY_SIZE)));

	free(hexstr);
	fclose(file);

	if (__ksz != NULL)
	{
		*__ksz = BINKEY_SIZE;
	}

	assert(__key != NULL);
	*__key = binkey;

	return PK_SUCCESS;
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

bool is_db_decrypted(sqlite3 *db)
{
	return sqlite3_exec(db, "SELECT count(*) FROM sqlite_master;", NULL, NULL, NULL) == SQLITE_OK;
}

int create_record(sqlite3 *db, const app_option *appopt, char **message)
{
	const char *sql;
	sqlite3_stmt *stmt;

	sql = "INSERT INTO account "
		"(sitename, siteurl, username, password, authtext, bakcode, comment) "
		"VALUES"
		"(:sitename, :siteurl, :username, :password, :authtext, :bakcode, :comment)";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
	{
		return sqlite3_finalize(stmt);
	}

	int idx;
	const entry *iter, field_table[8] = {
		{ "", appopt->sitename },
		{ "", appopt->siteurl },
		{ "", appopt->username },
		{ "", appopt->password },
		{ "", appopt->authtext },
		{ "", appopt->bakcode },
		{ "", appopt->comment },
		{ NULL, NULL },
	};

	idx = 1;
	iter = field_table;
	while (iter->key != NULL)
	{
		if (sqlite3_bind_text(stmt, idx++, iter->value, -1, SQLITE_STATIC) != SQLITE_OK)
		{
			return sqlite3_finalize(stmt);
		}

		iter++;
	}

	if (sqlite3_step(stmt) == SQLITE_DONE)
	{
		assert(message != NULL);
		*message = substr(appopt->sitename, 0, 0);
	}

	return sqlite3_finalize(stmt);
}

int read_record(sqlite3 *db, const app_option *appopt, char __attribute__ ((unused)) **message)
{
	const char *sql;
	sqlite3_stmt *stmt;

	sql = "SELECT * FROM account WHERE sitename LIKE :sitename";
	if (sqlite3_prepare_v3(db, sql, -1, SQLITE_PREPARE_PERSISTENT, &stmt, NULL) != SQLITE_OK)
	{
		return sqlite3_finalize(stmt);
	}

	int rc;
	if (appopt->sitename == NULL)
	{
		rc = sqlite3_bind_text(stmt, 1, "%", 1, SQLITE_STATIC);
	}
	else
	{
		char *search_pattern = concat(appopt->sitename, "%");
		rc = sqlite3_bind_text(stmt, 1, search_pattern, -1, SQLITE_TRANSIENT);
		free(search_pattern);
	}

	if (rc != SQLITE_OK)
	{
		return sqlite3_finalize(stmt);
	}

	recordqueue *q;
	recordfield *data;
	int field_maxlen_map[3] = { 0, 0, 0 };

	q = rcqmake();
	while (1)
	{
		rc = sqlite3_step(stmt);

		if (rc != SQLITE_ROW)
		{
			break;
		}

		data = rcfmake();

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

	debug_log("size of rcque is %lu\n", rcque_size);

	stringbuffer *buf;
	char *padstr;

	buf = sballoc(200);
	padstr = mkspase(appopt->wrap_threshold);

	int is_init;

	is_init = 1;
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
			sbfree(buf);
			buf = sballoc(200);
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

	debug_log("strbuffer resize executed %d times\n", sb_resize_count);

	return sqlite3_finalize(stmt);
}

void assign_field(sqlite3_stmt *stmt, int column, char **field, int *flen)
{
	const char *tmp_field = (const char *)sqlite3_column_text(stmt, column);
	int tmp_flen = sqlite3_column_bytes(stmt, column) / sizeof(char);

	if (tmp_field == NULL)
	{
		tmp_field = "(null)";
		tmp_flen = 6;
	}

	*field = substr(tmp_field, 0, 0);

	if (flen != NULL)
	{
		*flen = tmp_flen;
	}
}

void assign_by_large_value(int *dest, int tar)
{
	if (tar > *dest)
	{
		*dest = tar;
	}
}

int align_and_wrap_field(stringbuffer *buf, const char *field, int field_crtlen, int field_maxlen, int wrap_threshold, const char *padstr)
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
		field_fragment = substr(field, 0, substr_length);
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

void print_brief_field(stringbuffer *buf, recordfield *data, int field_maxlen_map[3], int wrap_threshold, const char *padstr)
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

void print_verbose_field(stringbuffer *buf, const recordfield *data, int *is_init)
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
		STRINGIFY(data->siteurl),
		STRINGIFY(data->username),
		STRINGIFY(data->password),
		STRINGIFY(data->authtext),
		STRINGIFY(data->bakcode),
		STRINGIFY(data->comment),
		STRINGIFY(data->sqltime),
		STRINGIFY(data->modtime));
}

int update_record(sqlite3 *db, const app_option *appopt, char **message)
{
	int rc, idx;
	sqlite3_stmt *stmt;

	stringbuffer *buf;

	const entry *iter, entries[8] = {
		{ "sitename", appopt->sitename },
		{ "siteurl", appopt->siteurl },
		{ "username", appopt->username },
		{ "password", appopt->password },
		{ "authtext", appopt->authtext },
		{ "bakcode", appopt->bakcode },
		{ "comment", appopt->comment },
		{ NULL, NULL },
	};

	buf = sballoc(200);
	iter = entries;
	while (iter->key != NULL)
	{
		sbprintf(buf, iter->value == NULL ? ", %s = %s" : ", %s = :%s", iter->key, iter->key);
		iter++;
	}

	char *update_column;
	int sql_offset;

	sql_offset = buf->size;
	update_column = substr(buf->data, 2, buf->size - 2);
	sbprintf(buf, "UPDATE account SET %s WHERE id = :id RETURNING sitename", update_column); /* skip first comma */
	free(update_column);

	rc = sqlite3_prepare_v2(db, buf->data + sql_offset, buf->size - sql_offset, &stmt, NULL);
	sbfree(buf);

	if (rc != SQLITE_OK)
	{
		return sqlite3_finalize(stmt);
	}

	iter = entries;
	idx = 1;
	while (iter->key != NULL)
	{
		if (iter->value == NULL)
		{
			iter++;
			continue;
		}

		if (sqlite3_bind_text(stmt, idx++, *(const char *)iter->value == '\0' ? NULL : iter->value, -1, SQLITE_STATIC) != SQLITE_OK)
		{
			return sqlite3_finalize(stmt);
		}

		iter++;
	}

	if (sqlite3_bind_int(stmt, idx, appopt->record_id) != SQLITE_OK)
	{
		return sqlite3_finalize(stmt);
	}

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		assert(message != NULL);
		*message = substr((const char *)sqlite3_column_text(stmt, 0), 0, 0);
	}

	return sqlite3_finalize(stmt);
}

int delete_record(sqlite3 *db, const app_option *appopt, char **message)
{
	const char *sql;
	sqlite3_stmt *stmt;

	sql = "DELETE FROM account WHERE id = :id RETURNING sitename";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
	{
		return sqlite3_finalize(stmt);
	}

	if (sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, ":id"), appopt->record_id) != SQLITE_OK)
	{
		return sqlite3_finalize(stmt);
	}

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		assert(message != NULL);
		*message = substr((const char *)sqlite3_column_text(stmt, 0), 0, 0);
	}

	return sqlite3_finalize(stmt);
}