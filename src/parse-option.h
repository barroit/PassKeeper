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

#ifndef PARSEOPT_H
#define PARSEOPT_H

enum option_type
{
	/* special types */
	OPTION_END,
	OPTION_GROUP,
	/* options with no arguments */
	OPTION_INTEGER,
	OPTION_COUNTUP,
	/* options with arguments */
	OPTION_STRING,
	OPTION_FILENAME,
};

enum option_flag
{
	OPTION_OPTARG   = 1 << 0,
	OPTION_NOARG    = 1 << 1,
	OPTION_ALLONEG  = 1 << 2,
	OPTION_SHOWARGH = 1 << 3,
	OPTION_REALPATH = 1 << 4,
	OPTION_UNSIGNED = 1 << 5,
	OPTION_CMDMODE  = 1 << 6,
	/**
	 * options with this flag will not shown in help messages
	 * but parser will parse the value for this option
	 */
	OPTION_HIDDEN   = 1 << 7,
	/**
	 * options with this flag will be ignored by parser and
	 * help messages get printed will not prefix with em dash
	 */
	OPTION_NOEMDASH = 1 << 8,
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
	enum option_flag flags;
};

enum command_parser_flag
{
	PARSER_ABORT_NON_OPTION = 1 << 0,
	PARSER_ONE_SHOT         = 1 << 1,
	PARSER_UNTIL_NON_OPTION = 1 << 2,
	PARSER_NO_SHORT_HELP    = 1 << 3, /* disable -h */
};

#define OPTION_END()					\
{							\
	.type = OPTION_END,				\
}

#define OPTION_GROUP(h)					\
{							\
	.type = OPTION_GROUP,				\
	.help = (h),					\
}

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

#define OPTION_FILENAME_F(s, l, v, a, h, f)		\
{							\
	.type  = OPTION_FILENAME,			\
	.alias = (s),					\
	.name  = (l),					\
	.value = (v),					\
	.argh  = (a),					\
	.help  = (h),					\
	.flags = (f),					\
}

#define OPTION_INTEGER_F(s, l, v, a, h, d, f)		\
{							\
	.type   = OPTION_INTEGER,			\
	.alias  = (s),					\
	.name   = (l),					\
	.value  = (v),					\
	.argh   = (a),					\
	.help   = (h),					\
	.defval = (d),					\
	.flags  = (f),					\
}

#define OPTION_SWITCH_F(s, l, v, h, f)			\
{							\
	.type   = OPTION_INTEGER,			\
	.alias  = (s),					\
	.name   = (l),					\
	.value  = (v),					\
	.help   = (h),					\
	.defval = 1,					\
	.flags  = OPTION_NOARG | OPTION_ALLONEG | (f),	\
}

#define OPTION_COUNTUP_F(s, l, v, h, f)			\
{							\
	.type   = OPTION_INTEGER,			\
	.alias  = (s),					\
	.name   = (l),					\
	.value  = (v),					\
	.help   = (h),					\
	.defval = 0,					\
	.flags  = OPTION_NOARG | (f),			\
}

#define OPTION_UNSIGNED_F(s, l, v, h, f)		\
{							\
	.type   = OPTION_INTEGER,			\
	.alias  = (s),					\
	.name   = (l),					\
	.value  = (v),					\
	.help   = (h),					\
	.defval = 0,					\
	.flags  = OPTION_UNSIGNED | (f),		\
}

#define OPTION_CMDMODE_F(s, l, v, h, d, f)		\
{							\
	.type   = OPTION_INTEGER,			\
	.alias  = (s),					\
	.name   = (l),					\
	.value  = (v),					\
	.help   = (h),					\
	.defval = (d),					\
	.flags  = OPTION_CMDMODE | OPTION_NOARG | (f),	\
}

#define OPTION_STRING(s, l, v, h)\
	OPTION_STRING_F((s), (l), (v), 0, (h), 0)

#define OPTION_SWITCH(s, l, v, h)\
	OPTION_SWITCH_F((s), (l), (v), (h), 0)

#define OPTION_COUNTUP(s, l, v, h)\
	OPTION_COUNTUP_F((s), (l), (v), (h), 0)

#define OPTION_UNSIGNED(s, l, v, h)\
	OPTION_UNSIGNED_F((s), (l), (v), (h), 0)

#define OPTION_CMDMODE(s, l, v, h, d)\
	OPTION_CMDMODE_F((s), (l), (v), (h), (d), 0)

#define OPTION_FILENAME(s, l, v, h)\
	OPTION_FILENAME_F((s), (l), (v), "path", (h), OPTION_SHOWARGH)

#define OPTION_PATHNAME(s, l, v, h)\
	OPTION_FILENAME_F((s), (l), (v), "file", (h), OPTION_REALPATH | OPTION_SHOWARGH)

#define OPTION_OPTARG(s, l, v, d, a, h)\
	OPTION_OPTARG_F((s), (l), (v), (d), (a), (h), OPTION_SHOWARGH)

#define OPTION_COMMAND(l, h)\
	OPTION_STRING_F(0, (l), 0, 0, (h), OPTION_NOARG | OPTION_NOEMDASH)

#define OPTION_CMDMODE(s, l, v, h, d)\
	OPTION_CMDMODE_F((s), (l), (v), (h), (d), 0)

#define OPTION__NANO(val)\
	OPTION_SWITCH('e', "nano", val, "use editor to edit records")

#define OPTION__CMDKEY(val)\
	OPTION_COUNTUP('k', "cmdkey", val, "input key from command line")

#ifndef DEFAULT_OPTMSG_ALIGNMENT
#define DEFAULT_OPTMSG_ALIGNMENT 23
#endif

extern int optmsg_alignment;

int parse_options(int argc, const char **argv, const char *prefix, const struct option *options, const char *const *usages, enum command_parser_flag flags);

#endif /* PARSEOPT_H */
