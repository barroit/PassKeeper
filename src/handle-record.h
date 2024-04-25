/****************************************************************************
**
** Copyright 2023, 2024 Jiamu Sun
** Contact: barroit@linux.com
**
** This file is part of PassKeeper.
**
** PassKeeper is free software: you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation, either version 3 of the License, or (at your
** option) any later version.
**
** PassKeeper is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with PassKeeper. If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef HANDLE_RECORD_H
#define HANDLE_RECORD_H

struct record
{
	const char *sitename;
	const char *siteurl;
	const char *username;
	const char *password;

	const char *guard;
	const char *recovery;
	const char *memo;

	const char *comment;
};

void populate_record_file(const char *rec_path, const struct record *rec);

extern bool is_blank_str(const char *str0);

static inline FORCEINLINE bool is_incomplete_record(const struct record *rec)
{
	return is_blank_str(rec->sitename) || is_blank_str(rec->password);
}

bool is_incomplete_record(const struct record *rec);

char *format_missing_field(const struct record *rec);

int read_record_file(struct record *rec, const char *rec_path);

static inline FORCEINLINE bool have_security_group(const struct record *rec)
{
	return rec->guard || rec->recovery || rec->memo;
}

static inline FORCEINLINE bool have_misc_group(const struct record *rec)
{
	return rec->comment || 0;
}

bool is_need_transaction(const struct record *rec);

void bind_record_basic_column(struct sqlite3_stmt *stmt, const struct record *rec);

void bind_record_security_column(struct sqlite3_stmt *stmt, int64_t account_id, const struct record *rec);

void bind_record_misc_column(struct sqlite3_stmt *stmt, int64_t account_id, const struct record *rec);

#endif /* HANDLE_RECORD_H */
