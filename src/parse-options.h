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

#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

enum option_type
{
	/* special types */
	OPTION_END,
	OPTION_GROUP,
	/* options with no arguments */
	OPTION_SWITCH,
	/* options with arguments */
	OPTION_STRING,
	OPTION_UNSIGNED,
	OPTION_FILENAME,
};

enum option_flag
{
	OPTION_OPTARG = 1 << 0,
	OPTION_NOARG = 1 << 1,
	OPTION_ALLONEG = 1 << 2,
	OPTION_SHOWARGH = 1 << 3,
	OPTION_REALPATH = 1 << 4,
	/**
	 * options with this flag will not shown in help messages
	 * but parser will parse the value for this option
	 */
	OPTION_HIDDEN = 1 << 5,
	/**
	 * options with this flag will be ignored by parser and
	 * help messages get printed will not prefix with em dash
	 */
	OPTION_NOEMDASH = 1 << 6,
};

struct option
{
	enum option_type type;

	char alias;
	const char *name;

	void *value;
	intptr_t defval;

	const char *help;
	const char *argh;

	unsigned count;
	unsigned flags;
};

enum option_parser_flag
{
	PARSER_ABORT_NON_OPTION = 1 << 0,
	PARSER_ONE_SHOT = 1 << 1,
	PARSER_STOP_AT_NON_OPTION = 1 << 2,
	/**
	 * disable -h
	 */
	PARSER_NO_SHORT_HELP = 1 << 3,
};

#define OPTION_STRING_F(s, l, v, a, h, f)		\
{							\
	.type  = OPTION_STRING,				\
	.alias = (s),					\
	.name  = (l),					\
	.value = (v),					\
	.argh  = (a),					\
	.help  = (h),					\
	.flags = (f),					\
}

#define OPTION_OPTARG_F(s, l, v, d, a, h, f)		\
{							\
	.type   = OPTION_STRING,			\
	.alias  = (s),					\
	.name   = (l),					\
	.value  = (v),					\
	.defval = (intptr_t)(d),			\
	.argh   = (a),					\
	.help   = (h),					\
	.flags  = OPTION_OPTARG | (f),			\
}

#define OPTION_FILENAME_F(s, l, v, h, a, f)		\
{							\
	.type  = OPTION_FILENAME,			\
	.alias = (s),					\
	.name  = (l),					\
	.value = (v),					\
	.help  = (h),					\
	.argh  = (a),					\
	.flags = (f),					\
}

#define OPTION_END()					\
{							\
	.type = OPTION_END,				\
}

#define OPTION_GROUP(h)					\
{							\
	.type = OPTION_GROUP,				\
	.help = (h),					\
}

#define OPTION_BOOLEAN_F(s, l, v, h, f)			\
{							\
	.type   = OPTION_SWITCH,			\
	.alias  = (s),					\
	.name   = (l),					\
	.value  = (v),					\
	.help   = (h),					\
	.defval = 1,					\
	.flags  = OPTION_NOARG | OPTION_ALLONEG | (f),	\
}

#define OPTION_UNSIGNED_F(s, l, v, a, h, f)		\
{							\
	.type  = OPTION_UNSIGNED,			\
	.alias = (s),					\
	.name  = (l),					\
	.value = (v),					\
	.argh  = (a),					\
	.help  = (h),					\
	.flags = (f),					\
}

#define OPTUINT_INIT (unsigned)~0
#define OPTUINT_UNCHANGED(v) ((v) == OPTUINT_INIT)

#define OPTION_STRING(s, l, v, h) OPTION_STRING_F((s), (l), (v), 0, (h), 0)
#define OPTION_HIDDEN_STRING(s, l, v) OPTION_STRING_F((s), (l), (v), 0, 0, OPTION_HIDDEN)

#define OPTION_BOOLEAN(s, l, v, h) OPTION_BOOLEAN_F((s), (l), (v), (h), 0)

#define OPTION_UNSIGNED(s, l, v, h) OPTION_UNSIGNED_F((s), (l), (v), 0, (h), 0)

#define OPTION_FILENAME(s, l, v, h) OPTION_FILENAME_F((s), (l), (v), (h), "path", OPTION_SHOWARGH)

#define OPTION_PATHNAME(s, l, v, h) OPTION_FILENAME_F((s), (l), (v), (h), "file", OPTION_REALPATH | OPTION_SHOWARGH)
#define OPTION_HIDDEN_PATHNAME(s, l, v) OPTION_FILENAME_F((s), (l), (v), 0, 0, OPTION_HIDDEN | OPTION_REALPATH)

#define OPTION_OPTARG(s, l, v, d, a, h) OPTION_OPTARG_F((s), (l), (v), (d), (a), (h), OPTION_SHOWARGH)
#define OPTION_HIDDEN_OPTARG(s, l, v, d) OPTION_OPTARG_F((s), (l), (v), (d), 0, 0, OPTION_HIDDEN)
#define OPTION_HIDDEN_OPTARG_ALLONEG(s, l, v, d) OPTION_OPTARG_F((s), (l), (v), (d), 0, 0, OPTION_HIDDEN | OPTION_ALLONEG)

#define OPTION_COMMAND(l, h) OPTION_STRING_F(0, (l), 0, 0, (h), OPTION_NOARG | OPTION_NOEMDASH)

#ifndef OPTION_USAGE_ALIGNMENT
#define OPTION_USAGE_ALIGNMENT 23
#endif

extern int option_usage_alignment;

int process_get_unsigned_argument_result(int rescode, const char *val, const char *field);

int parse_options(int argc, const char **argv, const char *prefix, const struct option *options, const char *const *usages, enum option_parser_flag flags);

extern const char *const cmd_count_usages[];
extern const char *const cmd_create_usages[];
extern const char *const cmd_delete_usages[];
extern const char *const cmd_help_usages[];
extern const char *const cmd_init_usages[];
extern const char *const cmd_makekey_usages[];
extern const char *const cmd_read_usages[];
extern const char *const cmd_update_usages[];
extern const char *const cmd_version_usages[];
extern const char *const cmd_pk_usages[];

extern const struct option cmd_count_options[];
extern const struct option cmd_create_options[];
extern const struct option cmd_delete_options[];
extern const struct option cmd_help_options[];
extern const struct option cmd_init_options[];
extern const struct option cmd_makekey_options[];
extern const struct option cmd_read_options[];
extern const struct option cmd_update_options[];
extern const struct option cmd_version_options[];
extern const struct option cmd_pk_options[];

#endif /* PARSE_OPTIONS_H */