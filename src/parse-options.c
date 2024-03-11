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

#include "parse-options.h"
#include "strbuf.h"
#include "strlist.h"

enum parse_option_result
{
	PARSING_HELP = -2,
	PARSING_ERROR = -1,
	PARSING_DONE = 0,
	PARSING_NON_OPTION,
	PARSING_UNKNOWN,
};

struct parser_context
{
	int raw_argc;
	int argc;
	const char **argv;

	unsigned parser_flags;

	unsigned argoc;
	const char **argov;

	const char *optstr;
};

enum option_parsed
{
	LONG_OPTION  = 1 << 0,
	SHORT_OPTION = 1 << 1,
	UNSET_OPTION = 1 << 2,
};

static const char *detailed_option(const struct option *opt, enum option_parsed flags)
{
	static struct strbuf sb = STRBUF_INIT;

	strbuf_reset(&sb);
	if (flags & SHORT_OPTION)
	{
		strbuf_printf(&sb, "switch `%c'", opt->alias);
	}
	else if (flags & UNSET_OPTION)
	{
		strbuf_printf(&sb, "option `no-%s'", opt->name);
	}
	else if (flags == LONG_OPTION)
	{
		strbuf_printf(&sb, "option `%s'", opt->name);
	}
	else
	{
		bug("detailed_option() got unknown flags %d", flags);
	}

	return sb.buf;
}

static enum parse_option_result assign_string_value(
	struct parser_context *ctx,
	const struct option *opt,
	enum option_parsed flags,
	const char **out)
{
	if (ctx->optstr)
	{
		*out = ctx->optstr;
		ctx->optstr = NULL;
	}
	else if (ctx->argc > 1)
	{
		ctx->argc--;
		*out = *++ctx->argv;
	}
	else
	{
		return error("%s requires a value", detailed_option(opt, flags));
	}

	return 0;
}

static enum parse_option_result assign_value(
	struct parser_context *ctx,
	const struct option *opt,
	enum option_parsed flags)
{
	switch (opt->type)
	{
		case OPTION_STRING:
			return assign_string_value(ctx, opt, flags, (const char **)opt->value);
		default:
			bug("opt->type %d should not happen", opt->type);
	}

	return 0;
}

static enum parse_option_result parse_long_option(
	struct parser_context *ctx,
	const char *arg,
	const struct option *options)
{
	while (options->type != OPTION_END)
	{
		const struct option *opt;
		const char *arg_rest;

		opt = options++;
		arg_rest = trim_prefix(arg, opt->name);

		if (arg_rest == NULL)
		{
			continue;
		}

		if (*arg_rest) /* option written in form xxx=xxx? */
		{
			if (*arg_rest != '=')
			{
				continue;
			}

			ctx->optstr = arg_rest + 1;
		}

		return assign_value(ctx, opt, LONG_OPTION);
	}

	ctx->optstr = arg;
	return PARSING_UNKNOWN;
}

static enum parse_option_result parse_short_option(
	struct parser_context *ctx,
	const struct option *options)
{
	while (options->type != OPTION_END)
	{
		if (options->alias == *ctx->optstr)
		{
			return assign_value(ctx, options, SHORT_OPTION);
		}

		options++;
	}

	return PARSING_UNKNOWN;
}

static enum parse_option_result parse_option_step(
	struct parser_context *ctx,
	const struct option *options)
{
	const char *argstr;

	argstr = *ctx->argv;
	/* check non options */
	if (*argstr != '-')
	{
		if (ctx->parser_flags & PARSER_STOP_AT_NON_OPTION)
		{
			return PARSING_NON_OPTION;
		}

		ctx->argov[ctx->argoc++] = ctx->argv[0];
	}

	/* lone -h asks for help */
	if (ctx->raw_argc == 1 && !strcmp(argstr + 1, "h"))
	{
		return PARSING_HELP;
	}

	/* check aliases */
	if (argstr[1] != '-')
	{
		ctx->optstr = argstr + 1; /* skip hyphen */

		while (ctx->optstr != 0)
		{
			switch (parse_short_option(ctx, options))
			{
				case PARSING_DONE:
					ctx->optstr++;
					break;
				case PARSING_UNKNOWN:
					return *ctx->optstr == 'h' ? PARSING_HELP : PARSING_UNKNOWN;
				case PARSING_ERROR:
					return PARSING_ERROR;
				case PARSING_HELP:
				case PARSING_NON_OPTION:
					bug("parse_short_option() cannot return these");
			}
		}

		return PARSING_DONE;
	}

	/* check end of option marker */
	if (argstr[2] == 0)
	{
		ctx->argc--;
		ctx->argv++;

		return PARSING_DONE;
	}

	/* check long options */
	switch (parse_long_option(ctx, argstr + 2, options))
	{
		case PARSING_DONE:
			break;
		case PARSING_ERROR:
			return PARSING_ERROR;
		case PARSING_HELP:
			return PARSING_HELP;
		case PARSING_UNKNOWN:
			return PARSING_UNKNOWN;
		case PARSING_NON_OPTION:
			bug("parse_long_option() cannot return these");
	}

	return PARSING_DONE;
}

static void prepare_context(
	struct parser_context *ctx,
	int argc, const char **argv,
	enum option_parser_flag flags)
{
	ctx->raw_argc = argc;
	ctx->argc = argc;
	ctx->argv = argv;

	ctx->parser_flags = flags;

	ctx->argoc = flags & PARSER_KEEP_ARGV0;
	ctx->argov = argv;
}

static inline void print_newline(FILE *stream)
{
	fputc('\n', stream);
}

static inline int indent_usage(FILE *stream)
{
	return fprintf(stream, "    ");
}

static int print_option_argh(const struct option *opt, FILE *stream)
{
	const char *fmt;
	bool printraw = (opt->flags & OPTION_RAWARGH) || !opt->argh || strpbrk(opt->argh, "()<>[]|");

	if (opt->flags & OPTION_OPTARG)
	{
		if (opt->name)
		{
			fmt = printraw ? "[=%s]" : "[=<%s>]";
		}
		else
		{
			fmt = printraw ? "[%s]" : "[<%s>]";
		}
	}
	else
	{
		fmt = printraw ? " %s" : " <%s>";
	}

	return fprintf(stream, fmt, opt->argh ? opt->argh : "...");
}

#ifndef OPTION_USAGE_WIDTH
#define OPTION_USAGE_WIDTH 26
#endif

static inline void pad_usage(FILE *stream, int pos)
{
	if (pos < OPTION_USAGE_WIDTH)
	{
		fprintf(stream, "%*s", OPTION_USAGE_WIDTH - pos, "");
	}
	else
	{
		fprintf(stream, "\n%*s", OPTION_USAGE_WIDTH, "");
	}
}

static int print_option_help(const struct option *opt, size_t pos, FILE *stream)
{
	const char *prev_line, *next_line;

	prev_line = opt->help ? opt->help : "";
	while (*prev_line)
	{
		next_line = strchrnul(prev_line, '\n');
		if (*next_line)
		{
			next_line++;
		}

		pad_usage(stream, pos);
		fwrite(prev_line, sizeof(char), next_line - prev_line, stream);

		pos = 0;
		prev_line = next_line;
	}

	return pos;
}

static inline bool has_option(const struct option *options, const char *name)
{
	while (options->type != OPTION_END && strcmp(name, options++->name));
	return options->type != OPTION_END;
}

static enum parse_option_result usage_with_options(
	const char *const *usages,
	const struct option *options,
	bool is_error)
{
	if (usages == NULL)
	{
		bug("usage_with_options() called without usages");
	}

	FILE *stream;
	const char *next_prefix, *usage_prefix, *or_prefix;
	struct strlist sl = STRLIST_INIT_DUP;
	size_t usage_length;
	bool println;
	const struct option *iter;

	stream = is_error ? stderr : stdout;
	usage_prefix = "usage: %s";
	or_prefix    = "   or: %s";
	next_prefix  = usage_prefix;
	usage_length = strlen(usage_prefix) - strlen("%s");

	while (*usages)
	{
		const char *ustr;
		size_t i;

		ustr = *usages++;
		string_list_split(&sl, ustr, '\n', -1);

		for (i = 0; i < sl.size; i++)
		{
			const char *line;

			line = sl.items[i].str;
			if (!i)
			{
				fprintf_ln(stream, next_prefix, line);
			}
			else
			{
				fprintf_ln(stream, "%*s", (int)usage_length, line);
			}
		}

		string_list_clear(&sl, false);
		next_prefix = or_prefix;
	}

	println = true;
	iter = options;

	while (iter->type != OPTION_END)
	{
		size_t prev_pos;
		const char *nn_name;

		if (iter->type == OPTION_GROUP)
		{
			print_newline(stream);
			println = 0;

			if (*iter->help)
			{
				fprintf(stream, "%s\n", iter->help);
			}

			continue;
		}

		if (println)
		{
			print_newline(stream);
			println = 0;
		}

		prev_pos = indent_usage(stream);

		if (iter->alias)
		{
			prev_pos += fprintf(stream, "-%c", iter->alias);
		}

		if (iter->name && iter->alias)
		{
			prev_pos += fprintf(stream, ", ");
		}

		if (iter->name)
		{
			bool is_nn_name = (iter->flags & OPTION_NONEG) || (nn_name = trim_prefix(iter->name, "no-")) != NULL;
			prev_pos += fprintf(stream, is_nn_name ? "--%s" : "--[no-]%s", iter->name);
		}

		if ((iter->flags & OPTION_RAWARGH) || !(iter->flags & OPTION_NONEG))
		{
			prev_pos += print_option_argh(iter, stream);
		}

		/* print help messages and reset position here */
		prev_pos = print_option_help(iter, prev_pos, stream);

		fputc('\n', stream);

		if (nn_name && !has_option(options, nn_name))
		{
			prev_pos = indent_usage(stream);
			prev_pos += fprintf(stream, "--%s", nn_name);

			pad_usage(stream, prev_pos);
			fprintf_ln(stream, "opposite of --no-%s", nn_name);
		}

		iter++;
	}

	fputc('\n', stream);

	return PARSING_HELP;
}

int parse_option(
	int argc, const char **argv,
	const struct option *options,
	const char *const *usages,
	enum option_parser_flag flags)
{
	struct parser_context *ctx = &(struct parser_context){ 0 };

	prepare_context(ctx, argc, argv, flags);

	while (ctx->argc)
	{
		switch (parse_option_step(ctx, options))
		{
			case PARSING_DONE:
				break;
			case PARSING_NON_OPTION:
				return ctx->argc;
			case PARSING_HELP:
				// fallthrough
			case PARSING_ERROR:
				exit(129);
			case PARSING_UNKNOWN:
				if (ctx->argv[0][1] == '-')
				{
					error("unknown option `%s'", ctx->argv[0] + 2);
				}
				else if (isascii(*ctx->optstr))
				{
					error("unknown switch `%c'", *ctx->optstr);
				}
				else
				{
					error("unknown non-ascii option in string: `%s'", *ctx->argv);
				}

				usage_with_options(usages, options, false);
				exit(129);
		}

		ctx->argc--;
		ctx->argv++;
	}

	return ctx->argc;
}