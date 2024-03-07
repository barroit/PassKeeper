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

#include "optparser.h"
#include "strbuf.h"

enum parse_option_result
{
	PARSING_HELP = -1,
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

	unsigned outarr_idx;
	const char **outarr;

	const char *optstr;
};

// enum option_parsed
// {
// 	LONG_OPTION  = 1 << 0,
// 	SHORT_OPTION = 1 << 1,
// 	UNSET_OPTION = 1 << 2,
// };

static enum parse_option_result assign_string_value(struct parser_context *ctx, const char **out)
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
		// return error(_("%s requires a value"), optname(opt, flags));
	}

	return 0;
}

static enum parse_option_result assign_value(struct parser_context *ctx, const struct option *opt)
{
	switch (opt->type)
	{
		case OPTION_END:
			abort();
		case OPTION_STRING:
			return assign_string_value(ctx, (const char **)opt->value);
	}

	return 0;
}

static enum parse_option_result parse_long_option(struct parser_context *ctx, const char *val, const struct option *options)
{
	while (options->type != OPTION_END)
	{
		const char *val_rest;

		val_rest = trim_prefix(val, options->name);
		if (val_rest == NULL)
		{
			continue;
		}

		if (*val_rest) /* option written in form xxx=xxx? */
		{
			if (*val_rest != '=')
			{
				continue;
			}

			ctx->optstr = val_rest + 1;
		}

		return assign_value(ctx, options);
	}

	return PARSING_UNKNOWN;
}

static enum parse_option_result parse_short_option(struct parser_context *ctx, const struct option *options)
{
	while (options->type != OPTION_END)
	{
		if (options->alias == *ctx->optstr)
		{
			return assign_value(ctx, options);
		}

		options++;
	}

	return PARSING_UNKNOWN;
}

static enum parse_option_result parse_option_step(struct parser_context *ctx, const struct option *options)
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

		ctx->outarr[ctx->outarr_idx++] = ctx->argv[0];
	}

	/* lone -h asks for help */
	if (ctx->raw_argc == 1 && !strcmp("h", argstr + 1))
	{
		goto show_usage;
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
					if (*ctx->optstr == 'h')
					{
						goto show_usage;
					}

					goto unknown;
				case PARSING_HELP:
				case PARSING_NON_OPTION:
					abort();
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
		case PARSING_HELP:
			goto show_usage;
		case PARSING_UNKNOWN:
			goto unknown;
		case PARSING_NON_OPTION:
			abort();
	}

	return PARSING_DONE;

show_usage:;

unknown:;

	return 0;
}

static void prepare_context(struct parser_context *ctx, int argc, const char **argv, enum option_parser_flag flags)
{
	ctx->raw_argc = argc;
	ctx->argc = argc;
	ctx->argv = argv;

	ctx->parser_flags = flags;

	ctx->outarr_idx = flags & PARSER_KEEP_ARGV0;
	ctx->outarr = argv;
}

int parse_option(int argc, const char **argv, const struct option *options, const char **usagestr, enum option_parser_flag flags)
{
	struct parser_context *ctx = &(struct parser_context){ 0 };

	prepare_context(ctx, argc, argv, flags);

	while (ctx->argc)
	{
		switch (parse_option_step(ctx, options))
		{
			case PARSING_HELP:
				puts(*usagestr);
				break;
			case PARSING_DONE:
			case PARSING_NON_OPTION:
			case PARSING_UNKNOWN:
				break;
		}

		ctx->argc--;
		ctx->argv++;
	}

	return ctx->argc;
}