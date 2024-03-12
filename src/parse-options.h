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
	// OPTION_INTEGER,
	// OPTION_FILENAME,
};

enum option_flag
{
	OPTION_OPTARG = 1 << 0,
	OPTION_NOARG = 1 << 1,
	OPTION_NONEG = 1 << 2,
	OPTION_RAWARGH = 1 << 3,
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
	PARSER_KEEP_ARGV0 = 1 << 0,
	PARSER_STOP_AT_NON_OPTION = 1 << 1,
};

#define OPTION_END()				\
{						\
	.type = OPTION_END,			\
}

#define OPTION_GROUP(h)				\
{						\
	.type = OPTION_TYPE_GROUP,		\
	.help = (h),				\
}

#define OPTION_BOOL(s, l, v, h)			\
{						\
	.type = OPTION_SWITCH,			\
	.alias = (s),				\
	.name = (l),				\
	.value = (v),				\
	.help = (h),				\
	.defval = 1,				\
	.flags = OPTION_NOARG,			\
}

#define OPTION_STRING_F(s, l, v, a, h, f)	\
{						\
	.type = OPTION_STRING,			\
	.alias = (s),				\
	.name = (l),				\
	.value = (v),				\
	.argh = (a),				\
	.help = (h),				\
	.flags = (f),				\
}

#define OPTION_STRING(s, l, v, a, h) OPTION_STRING_F(s, l, v, a, h, OPTION_NONEG)

int parse_option(int argc, const char **argv, const struct option *options, const char *const *usages, enum option_parser_flag flags);

#endif /* PARSE_OPTIONS_H */