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

#include "handle-record.h"
#include "strbuf.h"
#include "strlist.h"
#include "filesys.h"

#define SITENAME_ID ":sitename:"
#define SITEURL_ID  ":siteurl:"
#define USERNAME_ID ":username:"
#define PASSWORD_ID ":password:"
#define GUARD_ID    ":guard:"
#define RECOVERY_ID ":recovery:"
#define MEMO_ID     ":memo:"
#define COMMENT_ID  ":comment:"

static char *format_recfile_content(const struct record *rec, size_t *outlen)
{
	struct
	{
		const char *key;
		const char *val;
	} fmap0[] = {
		{ SITENAME_ID, rec->sitename },
		{ SITEURL_ID,  rec->siteurl  },
		{ USERNAME_ID, rec->username },
		{ PASSWORD_ID, rec->password },

		{ GUARD_ID,    rec->guard    },
		{ RECOVERY_ID, rec->recovery },
		{ MEMO_ID,     rec->memo     },

		{ COMMENT_ID,  rec->comment  },

		{ NULL },
	}, *fmap;
	struct strbuf message = STRBUF_INIT;

	fmap = fmap0;
	while (fmap->key != NULL)
	{
		strbuf_puts(&message, fmap->key);

		if (!is_blank_str(fmap->val))
		{
			strbuf_puts(&message, fmap->val);
		}
		else
		{
			strbuf_putchar(&message, '\n');
		}

		strbuf_putchar(&message, '\n');
	}

	strbuf_puts(&message, COMMON_RECORD_MESSAGE);

	if (message.length == 0)
	{
		bug("COMMON_RECORD_MESSAGE is empty");
	}

	*outlen = message.length;
	/**
	 * return this is fine since we add a COMMON_RECORD_MESSAGE to
	 * strbuf, it's impossible that message.buf reference to the
	 * default internal buffer of strbuf
	 */
	return message.buf;
}

void populate_record_file(const char *rec_path, const struct record *rec)
{ 
	char *message;
	size_t length;

	message = format_recfile_content(rec, &length);
	populate_file(rec_path, message, length);

	free(message);
}

static bool recfile_line_filter(struct strlist_elem *el)
{
	return *el->str != 0 && *el->str != '#';
}

static bool recfile_line_filter_all(struct strlist_elem *el)
{
	return recfile_line_filter(el) && *el->str != ':';
}

static void reassign_record(struct record *rec, struct strlist *lines)
{
	memset(rec, 0, sizeof(struct record));

	size_t i, ii;
	struct
	{
		const char *key;
		const char **value;
	} fmap0[] = {
		{ SITENAME_ID, &rec->sitename },
		{ SITEURL_ID,  &rec->siteurl  },
		{ USERNAME_ID, &rec->username },
		{ PASSWORD_ID, &rec->password },

		{ GUARD_ID,    &rec->guard    },
		{ RECOVERY_ID, &rec->recovery },
		{ MEMO_ID,     &rec->memo     },

		{ COMMENT_ID,  &rec->comment  },

		{ NULL },
	}, *fmap;
	struct strbuf *field = STRBUF_INIT_PTR;

	for (i = 0; i < lines->size; )
	{

		/**
		 * skip leading empty lines
		 */
		if (*lines->elvec[i].str != ':')
		{
			i++;
			continue;
		}

		fmap = fmap0;
		while (fmap->key != NULL)
		{
			if (!strcmp(lines->elvec[i].str, fmap->key))
			{
				break;
			}

			fmap++;
		}

		/**
		 * not a valid id
		 */
		if (fmap->key == NULL)
		{
			i++;
			continue;
		}

		ii = i + 1;
		while (ii < lines->size && *lines->elvec[ii].str != ':')
		{
			if (*lines->elvec[ii].str == '|')
			{
				strbuf_puts(field, lines->elvec[ii].str + 1);
			}
			else
			{
				strbuf_concat(field, lines->elvec[ii].str);
			}

			ii++;
		}

		if (ii == i + 1)
		{
			i++;
			continue;
		}

		i = ii;
		*fmap->value = strbuf_detach(field);

	}

	strbuf_destroy(field);
}

char *format_missing_field(const struct record *rec)
{
	const char *fmap0[2], **fmap;
	unsigned fcount;

	fcount = 0;
	fmap = fmap0;

	if (is_blank_str(rec->sitename))
	{
		fcount++;
		*fmap++ = "sitename";
	}
	else if (is_blank_str(rec->password))
	{
		fcount++;
		*fmap++ = "password";
	}
	else
	{
		bug("unknown condition");
	}

	fmap = fmap0;
	static char *buf;
	struct strlist *message = STRLIST_INIT_PTR_NODUP;

	switch (fcount)
	{
	case 2:
		strlist_push(message, *fmap++);
		strlist_push(message, "and");
		/* FALLTHRU */
	case 1:
		strlist_push(message, *fmap);
		break;
	default:
		bug("invalid field count '%d'", fcount);
	}

	buf = strlist_join(message, " ", EXT_JOIN_TAIL);
	strlist_destroy(message, false);

	return buf;
}

int read_record_file(struct record *rec, const char *rec_path)
{
	int rec_fd;
	char *rec_buf;
	off_t rec_bufsz;

	xio_pathname = rec_path;
	rec_fd = xopen(rec_path, O_RDONLY);

	if ((rec_bufsz = xlseek(rec_fd, 0, SEEK_END)) == 0)
	{
		close(rec_fd);
		goto cancelled;
	}

	xlseek(rec_fd, 0, SEEK_SET);

	rec_buf = xmalloc(rec_bufsz + 1);
	rec_buf[rec_bufsz] = 0;

	if (xread(rec_fd, rec_buf, rec_bufsz) == 0)
	{
		close(rec_fd);
		goto cancelled;
	}

	close(rec_fd);

	struct strlist *rec_line = STRLIST_INIT_PTR_DUPSTR;

	strlist_split(rec_line, rec_buf, '\n', -1);
	free(rec_buf);

	strlist_filter(rec_line, recfile_line_filter, false);

	if (rec_line->size == 0)
	{
		goto cancelled;
	}

	reassign_record(rec, rec_line);

	if (is_incomplete_record(rec))
	{
		strlist_filter(rec_line, recfile_line_filter_all, false);

		if (rec_line->size == 0)
		{
			goto cancelled;
		}
		else
		{
			return error("%s missing in the required fields",
					format_missing_field(rec));
		}
	}

	strlist_destroy(rec_line, false);

cancelled:
	puts("Creation is aborted due to empty record.");
	return 1;
}

bool is_need_transaction(struct record *rec)
{
	int group_count;

	group_count = 0;

	group_count += rec->sitename || rec->siteurl ||
			rec->username || rec->password;

	group_count += rec->guard || rec->recovery || rec->memo;

	group_count += rec->comment || 0;

	return group_count > 1;
}
