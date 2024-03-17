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
	OPTION_PATHNAME,
};

enum option_flag
{
	OPTION_OPTARG = 1 << 0,
	OPTION_NOARG = 1 << 1,
	OPTION_ALLONEG = 1 << 2,
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
	PARSER_ABORT_NON_OPTION = 1 << 0,
	PARSER_ONE_SHOT = 1 << 1,
};

#define OPTION_END()					\
{							\
	.type = OPTION_END,				\
}

#define OPTION_GROUP(h)					\
{							\
	.type = OPTION_TYPE_GROUP,			\
	.help = (h),					\
}

#define OPTION_BOOL(s, l, v, h)				\
{							\
	.type = OPTION_SWITCH,				\
	.alias = (s),					\
	.name = (l),					\
	.value = (v),					\
	.help = (h),					\
	.defval = 1,					\
	.flags = OPTION_NOARG | OPTION_ALLONEG,		\
}

#define OPTION_UNSIGNED(s, l, v, a, h)			\
{							\
	.type = OPTION_UNSIGNED,			\
	.alias = (s),					\
	.name = (l),					\
	.value = (v),					\
	.argh = (a),					\
	.help = (h),					\
}

#define OPTION_STRING(s, l, v, a, h)			\
{							\
	.type = OPTION_STRING,				\
	.alias = (s),					\
	.name = (l),					\
	.value = (v),					\
	.argh = (a),					\
	.help = (h),					\
}

#define OPTION_PATHNAME(s, l, v, h)			\
{							\
	.type = OPTION_PATHNAME,			\
	.alias = (s),					\
	.name = (l),					\
	.value = (v),					\
	.help = (h),					\
	.argh = ("path"),				\
}

extern int option_usage_width;

int print_help(const char *help, size_t pos, FILE *stream);

int parse_options(int argc, const char **argv, const char *prefix, const struct option *options, const char *const *usages, enum option_parser_flag flags);

extern const char *const cmd_count_usages[];
extern const char *const cmd_version_usages[];
extern const char *const cmd_delete_usages[];

extern const struct option cmd_count_options[];
extern const struct option cmd_version_options[];
extern const struct option cmd_delete_options[];

#endif /* PARSE_OPTIONS_H */