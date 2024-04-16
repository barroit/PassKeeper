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

#include "parseopt.h"
#include "strbuf.h"
#include "filesys.h"
#include "strlist.h"

enum parse_option_result
{
	PARSING_COMPLETE = -3,
	PARSING_HELP = -2,
	PARSING_ERROR = -1,
	PARSING_DONE = 0,
	PARSING_NON_OPTION,
	PARSING_UNKNOWN,
};

struct parser_context
{
	int argc;
	const char **argv;

	const char *prefix;
	enum option_parser_flag parser_flags;

	int argc0;
	unsigned idx;
	const char **out;

	const char *optstr;
};

enum option_parsed
{
	LONG_OPTION  = 0, /* use 0 for abbrev detection */
	SHORT_OPTION = 1 << 0,
	UNSET_OPTION = 1 << 1,
};

static const char *typed_option_name(const struct option *opt, enum option_parsed flags)
{
	static char ret[64];

	if (flags & SHORT_OPTION)
	{
		snprintf(ret, sizeof(ret), "switch '%c'", opt->alias);
	}
	else if (flags & UNSET_OPTION)
	{
		snprintf(ret, sizeof(ret), "option 'no-%s'", opt->name);
	}
	else if (flags == LONG_OPTION)
	{
		snprintf(ret, sizeof(ret), "option '%s'", opt->name);
	}
	else
	{
		bug("typed_option_name() got unknown flags %d", flags);
	}

	return ret;
}

static enum parse_option_result get_string_argument(
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
		return error("%s requires a value", typed_option_name(opt, flags));
	}

	return 0;
}

static void get_filename_argument(const char *prefix, const char **out)
{
	*out = prefix_filename(prefix, *out);
}

int process_get_unsigned_argument_result(int errcode, const char *val, const char *field)
{
	if (!errcode)
	{
		return 0;
	}

	if (errcode == ERANGE)
	{
		die("numerical value '%.9s%s' is too long", val, strlen(val) > 9 ? "..." : "");
	}

	return error("%s expects a numerical value", field);
}

static enum parse_option_result get_unsigned_argument(
	struct parser_context *ctx,
	const struct option *opt,
	enum option_parsed flags,
	unsigned *out)
{
	int rescode;
	const char *arg;

	if (ctx->optstr)
	{
		arg = ctx->optstr;
		ctx->optstr = NULL;
		rescode = strtou(arg, out);
	}
	else if (ctx->argc > 1)
	{
		ctx->argc--;
		arg = *++ctx->argv;
		rescode = strtou(arg, out);
	}
	else
	{
		return error("%s requires a value", typed_option_name(opt, flags));
	}

	return process_get_unsigned_argument_result(rescode, arg, typed_option_name(opt, flags));
}

static enum parse_option_result get_argument(
	struct parser_context *ctx,
	const struct option *opt,
	enum option_parsed flags)
{
	bool unset = flags & UNSET_OPTION;
	enum parse_option_result errcode;

	if (unset && ctx->optstr)
	{
		return error("%s takes no value", typed_option_name(opt, flags));
	}
	if (unset && !(opt->flags & OPTION_ALLONEG))
	{
		return error("%s isn't available", typed_option_name(opt, flags));
	}
	if (!(flags & SHORT_OPTION) && ctx->optstr && (opt->flags & OPTION_NOARG))
	{
		return error("%s takes no value", typed_option_name(opt, flags));
	}

	switch (opt->type)
	{
		case OPTION_SWITCH:
			*(int *)opt->value = unset ? 0 : opt->defval;
			break;
		case OPTION_STRING:
			if (unset)
			{
				*(const char **)opt->value = NULL;
			}
			else if ((opt->flags & OPTION_OPTARG) && !ctx->optstr)
			{
				*(const char **)opt->value = (const char *)opt->defval;
			}
			else
			{
				return get_string_argument(ctx, opt, flags, (const char **)opt->value);
			}

			break;
		case OPTION_FILENAME:
			errcode = get_string_argument(ctx, opt, flags, (const char **)opt->value);
			if (errcode)
			{
				return errcode;
			}

			get_filename_argument(ctx->prefix, (const char **)opt->value);
			break;
		case OPTION_UNSIGNED:
			return get_unsigned_argument(ctx, opt, flags, (unsigned *)opt->value);
		default:
			bug("opt->type %d should not happen", opt->type);
	}

	return 0;
}

static enum parse_option_result parse_long_option(
	struct parser_context *ctx,
	const char *argstr,
	const struct option *options,
	const struct option **outopt)
{
	const struct option *abbrev_option, *ambiguous_option;
	enum option_parsed abbrev_flags, ambiguous_flags;
	const char *argstr_end;

	abbrev_option = NULL;
	ambiguous_option = NULL;
	abbrev_flags = LONG_OPTION;
	ambiguous_flags = LONG_OPTION;
	argstr_end = strchrnul(argstr, '=');

	while (options->type != OPTION_END)
	{
		if (options->type == OPTION_GROUP || options->flags & OPTION_NOEMDASH)
		{
			options++;
			continue;
		}

		const struct option *opt;
		enum option_parsed flags, opt_flags;
		const char *argstr_rest, *opt_name;

		opt = options++;
		opt_name = opt->name;
		flags = LONG_OPTION;
		opt_flags = LONG_OPTION;

		if (!starts_with(argstr, "no-") && (opt->flags & OPTION_ALLONEG) && skip_prefix(opt_name, "no-", &opt_name))
		{
			opt_flags |= UNSET_OPTION;
		}

		if (!skip_prefix(argstr, opt_name, &argstr_rest))
		{
			argstr_rest = NULL;
		}

		if (argstr_rest == NULL)
		{
			/* abbreviated? */
			if (!strncmp(opt_name, argstr, argstr_end - argstr))
			{
is_abbreviated:
				if (abbrev_option != NULL)
				{
					ambiguous_option = abbrev_option;
					ambiguous_flags = abbrev_flags;
				}

				if (!(flags & UNSET_OPTION) && *argstr_end)
				{
					ctx->optstr = argstr_end + 1;
				}

				abbrev_option = opt;
				abbrev_flags = flags ^ opt_flags;

				continue;
			}

			/* negation allowed? */
			if (!(opt->flags & OPTION_ALLONEG))
			{
				continue;
			}

			/* negated and abbreviated very much? */
			if (starts_with("no-", argstr))
			{
				flags |= UNSET_OPTION;
				goto is_abbreviated;
			}

			/* negated? */
			if (!starts_with(argstr, "no-"))
			{
				continue;
			}

			flags |= UNSET_OPTION;
			if (!skip_prefix(argstr + 3, opt_name, &argstr_rest))
			{
				/* abbreviated and negated? */
				if (starts_with(opt_name, argstr + 3))
				{
					goto is_abbreviated;
				}
				else
				{
					continue;
				}
			}
		}

		if (*argstr_rest) /* option written in form xxx=xxx? */
		{
			if (*argstr_rest != '=')
			{
				continue;
			}

			ctx->optstr = argstr_rest + 1;
		}

		*outopt = opt;
		return get_argument(ctx, opt, flags ^ opt_flags);
	}

	if (ambiguous_option)
	{
		error("ambiguous option: %s (could be --%s%s or --%s%s)",
			argstr,
			(ambiguous_flags & UNSET_OPTION) ?  "no-" : "",
			ambiguous_option->name,
			(abbrev_flags & UNSET_OPTION) ?  "no-" : "",
			abbrev_option->name);

		return PARSING_HELP;
	}

	if (abbrev_option)
	{
		*outopt = abbrev_option;
		return get_argument(ctx, abbrev_option, abbrev_flags);
	}

	ctx->optstr = argstr;
	return PARSING_UNKNOWN;
}

static enum parse_option_result parse_short_option(
	struct parser_context *ctx,
	const struct option *options,
	const struct option **outopt)
{
	enum parse_option_result rescode;
	while (options->type != OPTION_END)
	{
		if (options->alias == *ctx->optstr)
		{
			*outopt = options;
			rescode = get_argument(ctx, options, SHORT_OPTION);
			return ctx->optstr == NULL ? PARSING_COMPLETE: rescode;
		}

		options++;
	}

	return PARSING_UNKNOWN;
}

static enum parse_option_result validate_parsed_value(const struct option *opt)
{
	if (opt->flags & OPTION_REALPATH)
	{
		struct stat st;
		if (stat(*(const char **)opt->value, &st))
		{
			return error("'%s' did not match any files", *(const char **)opt->value);
		}

		if (S_ISDIR(st.st_mode))
		{
			return error("'%s' is not a regular file", *(const char **)opt->value);
		}
	}

	return 0;
}

static enum parse_option_result parse_option_next(
	struct parser_context *ctx,
	const struct option *options)
{
	const char *argstr;
	enum parse_option_result rescode;
	bool allow_short_help;

	argstr = *ctx->argv;
	allow_short_help = !(ctx->parser_flags & PARSER_NO_SHORT_HELP);

	if ((ctx->parser_flags & PARSER_ONE_SHOT) && ctx->argc0 - ctx->argc == 1)
	{
		return PARSING_COMPLETE;
	}

	/* check non options */
	if (*argstr != '-')
	{
		if (ctx->parser_flags & PARSER_ABORT_NON_OPTION)
		{
			return PARSING_NON_OPTION;
		}

		if (ctx->parser_flags & PARSER_STOP_AT_NON_OPTION)
		{
			return PARSING_COMPLETE;
		}

		ctx->out[ctx->idx++] = *ctx->argv;
		return PARSING_DONE;
	}

	/* lone -h asks for help */
	if (ctx->argc0 == 1 && !strcmp(argstr + 1, "h") && allow_short_help)
	{
		return PARSING_HELP;
	}

	const struct option *outopt;

	/* check aliases */
	if (argstr[1] != '-')
	{
		/* skip hyphen */
		ctx->optstr = argstr + 1;

		while (ctx->optstr != 0)
		{
			switch ((rescode = parse_short_option(ctx, options, &outopt)))
			{
				case PARSING_DONE:
					ctx->optstr++;
					/* FALLTHRU */
				case PARSING_COMPLETE:
					break;
				case PARSING_UNKNOWN:
					if (*ctx->optstr == 'h' && allow_short_help)
					{
						return PARSING_HELP;
					}
					return PARSING_UNKNOWN;
				case PARSING_ERROR:
					return PARSING_ERROR;
				default:
					bug("parse_short_option() cannot return status %d", rescode);
			}
		}

		goto finish;
	}

	/* check end of option marker */
	if (argstr[2] == 0)
	{
		ctx->argc--;
		ctx->argv++;

		return PARSING_COMPLETE;
	}

	if (!strcmp(argstr + 2, "help"))
	{
		return PARSING_HELP;
	}

	/* check long options */
	switch ((rescode = parse_long_option(ctx, argstr + 2, options, &outopt)))
	{
		case PARSING_DONE:
			break;
		case PARSING_ERROR:
			return PARSING_ERROR;
		case PARSING_HELP:
			return PARSING_HELP;
		case PARSING_UNKNOWN:
			return PARSING_UNKNOWN;
		default:
			bug("parse_long_option() cannot return status %d", rescode);
	}

finish:
	return validate_parsed_value(outopt);
}

static void prepare_context(
	struct parser_context *ctx,
	int argc, const char **argv,
	const char *prefix,
	enum option_parser_flag flags)
{
	ctx->argc0 = argc;
	ctx->argc = argc;
	ctx->argv = argv;
	ctx->prefix = prefix;

	ctx->parser_flags = flags;

	ctx->out = argv;
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
	bool printraw = !opt->argh || strpbrk(opt->argh, "()<>[]|");

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

int option_usage_alignment = OPTION_USAGE_ALIGNMENT;

static void pad_usage(FILE *stream, int pos)
{
	if (pos < option_usage_alignment)
	{
		fprintf(stream, "%*s", option_usage_alignment - pos, "");
	}
	else
	{
		fprintf(stream, "\n%*s", option_usage_alignment, "");
	}
}

static int print_help(const char *help, size_t pos, FILE *stream)
{
	const char *prev_line, *next_line;

	prev_line = help;
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

static inline int print_option_help(const struct option *opt, size_t pos, FILE *stream)
{
	return print_help(opt->help ? opt->help : "", pos, stream);
}

static inline bool has_option(const struct option *options, const char *name)
{
	while (options->type != OPTION_END && strcmp(name, options->name))
	{
		options++;
	}

	return options->type != OPTION_END;
}

static enum parse_option_result usage_with_options(
	const char *const *usages,
	const struct option *options,
	bool is_error)
{
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
		strlist_split(&sl, ustr, '\n', -1);

		for (i = 0; i < sl.size; i++)
		{
			const char *line;

			line = sl.elvec[i].str;
			if (!i)
			{
				fprintfln(stream, next_prefix, line);
			}
			else
			{
				fprintfln(stream, "%*s%s", (int)usage_length, "", line);
			}
		}

		strlist_clear(&sl, false);
		next_prefix = or_prefix;
	}

	println = true;
	iter = options;

	while (iter->type != OPTION_END)
	{
		if (iter->flags & OPTION_HIDDEN)
		{
			iter++;
			continue;
		}

		size_t prev_pos;
		const char *negpos_name;

		negpos_name = NULL;
		if (iter->type == OPTION_GROUP)
		{
			print_newline(stream);
			println = 0;

			if (*iter->help)
			{
				fprintf(stream, "%s\n", iter->help);
			}

			iter++;
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
			bool skip_emdash;

			skip_emdash = iter->flags & OPTION_NOEMDASH;
			if (!(iter->flags & OPTION_ALLONEG) || skip_prefix(iter->name, "no-", &negpos_name))
			{
				prev_pos += fprintf(stream, "%s%s", skip_emdash ? "" : "--", iter->name);
			}
			else
			{
				prev_pos += fprintf(stream, "%s[no-]%s", skip_emdash ? "" : "--", iter->name);
			}
		}

		if ((iter->flags & OPTION_SHOWARGH) && !(iter->flags & OPTION_NOARG))
		{
			prev_pos += print_option_argh(iter, stream);
		}

		/* print help messages and reset position here */
		prev_pos = print_option_help(iter, prev_pos, stream);

		fputc('\n', stream);

		if (negpos_name && !has_option(options, negpos_name))
		{
			prev_pos = indent_usage(stream);
			prev_pos += fprintf(stream, "--%s", negpos_name);

			pad_usage(stream, prev_pos);
			fprintfln(stream, "opposite of --no-%s", negpos_name);
		}

		iter++;
	}

	fputc('\n', stream);

	return PARSING_HELP;
}

int parse_options(
	int argc, const char **argv,
	const char *prefix,
	const struct option *options,
	const char *const *usages,
	enum option_parser_flag flags)
{
	struct parser_context *ctx = &(struct parser_context){ 0 };

	prepare_context(ctx, argc, argv, prefix, flags);

	while (ctx->argc)
	{
		switch (parse_option_next(ctx, options))
		{
			case PARSING_DONE:
				break;
			case PARSING_COMPLETE:
				goto finish;
			case PARSING_NON_OPTION:
				error("unknown argument '%s'", *ctx->argv);
				exit(129);
			case PARSING_HELP:
				usage_with_options(usages, options, false);
				/* FALLTHRU */
			case PARSING_ERROR:
				exit(129);
			case PARSING_UNKNOWN:
				if (ctx->argv[0][1] == '-')
				{
					error("unknown option '%s'", ctx->argv[0] + 2);
				}
				else if (isascii(*ctx->optstr))
				{
					error("unknown switch '%c'", *ctx->optstr);
				}
				else
				{
					error("unknown non-ascii option in string: '%s'", *ctx->argv);
				}

				usage_with_options(usages, options, true);
				exit(129);
		}

		ctx->argc--;
		ctx->argv++;
	}

finish:
	if (ctx->argc)
	{
		MOVE_ARRAY(ctx->out + ctx->idx, ctx->argv, ctx->argc);
	}

	ctx->out[ctx->idx + ctx->argc] = NULL;
	return ctx->idx + ctx->argc;
}