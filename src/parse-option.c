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

#include "parse-option.h"
#include "strbuf.h"
#include "filesys.h"
#include "strlist.h"

enum parse_result
{
	PARSING_COMPLETE = -3,
	PARSING_HELP     = -2,
	PARSING_ERROR    = -1, /**
				* this value must match
				* return value of error()
				*/
	PARSING_DONE     = 0,
	PARSING_NON_OPTION,
	PARSING_UNKNOWN,
};

enum option_category
{
	LONG_OPTION  = 0,
	SHORT_OPTION = 1 << 0,
	UNSET_OPTION = 1 << 1,
};

struct command_mode
{
	int val, *valptr;
	const struct option *opt;
	enum option_category category;

	struct command_mode *next;
};

struct parser_context
{
	int argc;
	const char **argv;

	const char *prefix;
	enum command_parser_flag flags;

	int argc0;
	unsigned idx;
	const char **out;

	const char *optstr;

	struct command_mode *cmdmode;
};

static void sem_optname(
	char *buf, size_t size,
	const struct option *opt, enum option_category category)
{
	if (category & SHORT_OPTION)
	{
		snprintf(buf, size, "switch ‘%c’", opt->alias);
	}
	else if (category & UNSET_OPTION)
	{
		snprintf(buf, size, "option ‘no-%s’", opt->name);
	}
	else if (category == LONG_OPTION)
	{
		snprintf(buf, size, "option ‘%s’", opt->name);
	}
	else
	{
		bug("sem_optname() got unknown option category "
			"‘%d’", category);
	}
}

static enum parse_result get_arg_str(
	struct parser_context *ctx,
	const struct option *opt,
	enum option_category category,
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
		char optname[64];
		sem_optname(optname, sizeof(optname), opt, category);

		return error("%s requires a value", optname);
	}

	return 0;
}

static enum parse_result get_arg_uint(
	struct parser_context *ctx,
	const struct option *opt,
	enum option_category category,
	unsigned *res)
{
	const char *arg;
	char optname[64];

	if (ctx->optstr)
	{
		arg = ctx->optstr;
		ctx->optstr = NULL;
	}
	else if (ctx->argc > 1)
	{
		ctx->argc--;
		arg = *++ctx->argv;
	}
	else
	{
		sem_optname(optname, sizeof(optname), opt, category);

		return error("%s requires a value", optname);
	}

	if (strtou(arg, res) == 0)
	{
		if (errno == ERANGE)
		{
			die("numerical value ‘%.9s%s’ is too long",
				arg, strlen(arg) > 9 ? "..." : "");
		}
		else
		{
			sem_optname(optname, sizeof(optname),
					opt, category);

			return error("%s expects a numerical value", optname);
		}
	}

	return 0;
}

static enum parse_result get_arg_routine(
	struct parser_context *ctx,
	const struct option *opt,
	enum option_category category)
{
	enum parse_result rescode;
	bool unset;
	char optname[64];
	const char **strval;

	unset = category & UNSET_OPTION;
	if (unset && ctx->optstr)
	{
		sem_optname(optname, sizeof(optname), opt, category);
		return error("%s takes no value", optname);
	}

	if (unset && !(opt->flags & OPTION_ALLONEG))
	{
		sem_optname(optname, sizeof(optname), opt, category);
		return error("%s isn't available", optname);
	}

	if (!(category & SHORT_OPTION) && ctx->optstr &&
		(opt->flags & OPTION_NOARG))
	{
		sem_optname(optname, sizeof(optname), opt, category);
		return error("%s takes no value", optname);
	}

	switch (opt->type)
	{
	case OPTION_INTEGER:
		if (opt->flags & OPTION_UNSIGNED)
		{
			return get_arg_uint(ctx, opt, category,
						(unsigned *)opt->value);
		}
		else
		{
			*(int *)opt->value = unset ? 0 : opt->defval;
		}

		return 0;
	case OPTION_COUNTUP:
		(*(int *)opt->value)++;

		return 0;
	case OPTION_STRING:
		strval = (const char **)opt->value;

		if (unset)
		{
			*strval = NULL;
		}
		else if ((opt->flags & OPTION_OPTARG) && !ctx->optstr)
		{
			*strval = (const char *)opt->defval;
		}
		else
		{
			return get_arg_str(ctx, opt, category, strval);
		}

		return 0;
	case OPTION_FILENAME:
		strval = (const char **)opt->value;

		if ((rescode = get_arg_str(ctx, opt, category, strval)) != 0)
		{
			return rescode;
		}
		*strval = prefix_filename(ctx->prefix, *strval);

		return 0;
	default:
		bug("opt->type shall not be ‘%d’", opt->type);
	}
}

static void prefix_optname(
	char *buf, size_t size,
	const struct option *opt, enum option_category category)
{
	if (category & SHORT_OPTION)
	{
		snprintf(buf, size, "-%c", opt->alias);
	}
	else
	{
		snprintf(buf, size, "--%s%s",
			  category & UNSET_OPTION ? "no-" : "", opt->name);
	}
}

static int verify_cmdmode(
	struct command_mode *head,
	const struct option *opt,
	enum option_category category)
{
	struct command_mode *iter;

	list_for_each(iter, head)
	{
		if (*iter->valptr == iter->val)
		{
			continue;
		}

		if (iter->opt &&
			((iter->opt->flags | opt->flags) & OPTION_CMDMODE))
		{
			break;
		}

		iter->opt = opt;
		iter->category = category;
		iter->val = *iter->valptr;
	}

	if (iter == NULL)
	{
		return 0;
	}

	char optname1[64], optname2[64];

	prefix_optname(optname1, sizeof(optname1), opt, category);
	prefix_optname(optname2, sizeof(optname2), iter->opt, iter->category);

	return error("options ‘%s’ and ‘%s’ cannot be used together",
			optname1, optname2);
}

static enum parse_result get_arg(
	struct parser_context *ctx,
	const struct option *opt,
	enum option_category category)
{
	enum parse_result rescode;

	if ((rescode = get_arg_routine(ctx, opt, category)) != 0)
	{
		return rescode;
	}

	if (ctx->cmdmode != NULL &&
	     verify_cmdmode(ctx->cmdmode, opt, category) != 0)
	{
		return PARSING_ERROR;
	}

	return 0;
}

static enum parse_result parse_long_option(
	struct parser_context *ctx,
	const char *argstr,
	const struct option *iter,
	const struct option **parsed)
{
	const struct option  *abbrev_option,  *ambiguous_option;
	enum option_category abbrev_category, ambiguous_category;
	const char *argstr_end;

	abbrev_option = NULL;
	ambiguous_option = NULL;

	abbrev_category = LONG_OPTION;
	ambiguous_category = LONG_OPTION;

	argstr_end = strchrnul(argstr, '=');

	while (iter->type != OPTION_END)
	{
/* START LOOP */
	if (iter->type == OPTION_GROUP || iter->flags & OPTION_NOEMDASH)
	{
		iter++;
		continue;
	}

	const struct option *opt;
	enum option_category arg_category, opt_category;
	const char *argstr_rest, *optname;

	opt = iter++;
	optname = opt->name;
	
	arg_category = LONG_OPTION;
	opt_category = LONG_OPTION;

	if (!starts_with(argstr, "no-") &&
	     (opt->flags & OPTION_ALLONEG) &&
	      skip_prefix(optname, "no-", &optname))
	{
		opt_category |= UNSET_OPTION;
	}

	if (!skip_prefix(argstr, optname, &argstr_rest))
	{
		argstr_rest = NULL;
	}

	if (argstr_rest == NULL)
	{
		/* abbreviated? */
		if (!strncmp(optname, argstr, argstr_end - argstr))
		{
is_abbreviated:
			if (abbrev_option != NULL)
			{
				ambiguous_option = abbrev_option;
				ambiguous_category = abbrev_category;
			}

			if (!(arg_category & UNSET_OPTION) && *argstr_end)
			{
				ctx->optstr = argstr_end + 1;
			}

			abbrev_option = opt;
			abbrev_category = arg_category ^ opt_category;

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
			arg_category |= UNSET_OPTION;
			goto is_abbreviated;
		}

		/* negated? */
		if (!starts_with(argstr, "no-"))
		{
			continue;
		}

		arg_category |= UNSET_OPTION;
		if (!skip_prefix(argstr + 3, optname, &argstr_rest))
		{
			/* abbreviated and negated? */
			if (starts_with(optname, argstr + 3))
			{
				goto is_abbreviated;
			}
			else
			{
				continue;
			}
		}
	}

	/* option written in form xxx=xxx? */
	if (*argstr_rest)
	{
		if (*argstr_rest != '=')
		{
			continue;
		}

		ctx->optstr = argstr_rest + 1;
	}

	*parsed = opt;
	return get_arg(ctx, opt, arg_category ^ opt_category);
/* END LOOP */
	}

	if (ambiguous_option)
	{
		error("ambiguous option: %s (could be --%s%s or --%s%s)",
			argstr,
			 (ambiguous_category & UNSET_OPTION) ?  "no-" : "",
			  ambiguous_option->name,
			  (abbrev_category & UNSET_OPTION) ?  "no-" : "",
			   abbrev_option->name);

		return PARSING_HELP;
	}

	if (abbrev_option)
	{
		*parsed = abbrev_option;
		return get_arg(ctx, abbrev_option, abbrev_category);
	}

	ctx->optstr = argstr;
	return PARSING_UNKNOWN;
}

static enum parse_result parse_short_option(
	struct parser_context *ctx,
	const struct option *iter,
	const struct option **parsed)
{
	while (iter->type != OPTION_END)
	{
		if (iter->alias == *ctx->optstr)
		{
			ctx->optstr = ctx->optstr[1] ? ctx->optstr + 1 : NULL;

			*parsed = iter;
			return get_arg(ctx, iter, SHORT_OPTION);
		}

		iter++;
	}

	return PARSING_UNKNOWN;
}

static enum parse_result validate_parsed_value(const struct option *opt)
{
	if (opt->flags & OPTION_REALPATH)
	{
		struct stat st;
		const char *path;

		path = *(const char **)opt->value;
		if (stat(path, &st))
		{
			return error("‘%s’ did not match any files", path);
		}

		if (S_ISDIR(st.st_mode))
		{
			return error("‘%s’ is not a regular file", path);
		}
	}

	return 0;
}

static enum parse_result parse_option_next(
	struct parser_context *ctx,
	const struct option *options)
{
	const char *argstr;
	enum parse_result rescode;
	bool allow_short_help;

	argstr = *ctx->argv;
	allow_short_help = !(ctx->flags & PARSER_NO_SHORT_HELP);

	if ((ctx->flags & PARSER_ONE_SHOT) &&
		ctx->argc0 - ctx->argc == 1)
	{
		return PARSING_COMPLETE;
	}

	/* check non options */
	if (*argstr != '-')
	{
		if (ctx->flags & PARSER_ABORT_NON_OPTION)
		{
			return PARSING_NON_OPTION;
		}

		if (ctx->flags & PARSER_UNTIL_NON_OPTION)
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

	const struct option *parsed;

	/* check aliases */
	if (argstr[1] != '-')
	{
		/* skip hyphen */
		ctx->optstr = argstr + 1;

		while (ctx->optstr != NULL)
		{
			rescode = parse_short_option(ctx, options, &parsed);

			switch (rescode)
			{
			case PARSING_DONE:
				break;
			case PARSING_UNKNOWN:
				if (*ctx->optstr == 'h' && allow_short_help)
				{
					return PARSING_HELP;
				}

				/* FALLTHRU */
			case PARSING_ERROR:
				return rescode;
			default:
				bug("parse_short_option() shall not return "
					"‘%d’", rescode);
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
	rescode = parse_long_option(ctx, argstr + 2, options, &parsed);

	switch (rescode)
	{
	case PARSING_DONE:
		break;
	case PARSING_ERROR:
	case PARSING_HELP:
	case PARSING_UNKNOWN:
		return rescode;
	default:
		bug("parse_long_option() shall not return ‘%d’", rescode);
	}

finish:
	return validate_parsed_value(parsed);
}

#define INDENT_STRING "    "

int opt_argh_indent = DEFAULT_OPT_ARGH_INDENT;

#define pad_usage(stream, pos)								\
	do										\
	{										\
		if (pos < opt_argh_indent)						\
		{									\
			fprintf(stream, "%*s", opt_argh_indent - (int)pos, "");	\
		}									\
		else									\
		{									\
			fprintf(stream, "\n%*s", opt_argh_indent, "");			\
		}									\
	}										\
	while (0)

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

static enum parse_result usage_with_options(
	const char *const *usages,
	const struct option *options,
	bool is_error)
{
	FILE *stream;
	const char *next_prefix, *usage_prefix, *or_prefix;
	struct strlist *sl = STRLIST_INIT_PTR_DUPSTR;
	size_t usage_length;
	bool need_newline;
	const struct option *iter;

	stream       = is_error ? stderr : stdout;
	usage_prefix = "usage: %s";
	or_prefix    = "   or: %s";
	next_prefix  = usage_prefix;
	usage_length = strlen(usage_prefix) - strlen("%s");

	while (*usages)
	{
		const char *ustr;
		size_t i;

		ustr = *usages++;
		strlist_split(sl, ustr, '\n', -1);

		for (i = 0; i < sl->size; i++)
		{
			const char *line;

			line = sl->elvec[i].str;
			if (!i)
			{
				fprintf(stream, next_prefix, line);
				fputc('\n', stream);
			}
			else
			{
				fprintf(stream, "%*s%s\n",
					(int)usage_length, "", line);
			}
		}

		strlist_trunc(sl, false);
		next_prefix = or_prefix;
	}
	strlist_destroy(sl, false);

	need_newline = true;
	iter = options;

	while (iter->type != OPTION_END)
	{
/* START LOOP */
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
		fputc('\n', stream);
		need_newline = false;

		if (*iter->help)
		{
			fprintf(stream, "%s\n", iter->help);
		}

		iter++;
		continue;
	}

	if (need_newline)
	{
		fputc('\n', stream);
		need_newline = false;
	}

	prev_pos = fprintf(stream, INDENT_STRING);

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
		const char *prefix;

		prefix = (iter->flags & OPTION_NOEMDASH) ? "" : "--";
		prev_pos += fprintf(stream, "%s", prefix);

		if (!(iter->flags & OPTION_ALLONEG) ||
			skip_prefix(iter->name, "no-", &negpos_name))
		{
			prev_pos += fprintf(stream, "%s", iter->name);
		}
		else
		{
			prev_pos += fprintf(stream, "[no-]%s", iter->name);
		}
	}

	if ((iter->flags & OPTION_SHOWARGH) &&
		!(iter->flags & OPTION_NOARG))
	{
		prev_pos += print_option_argh(iter, stream);
	}

	/* print help messages and reset position here */
	prev_pos = print_help(iter->help ? iter->help : "", prev_pos, stream);

	fputc('\n', stream);

	if (negpos_name)
	{
		while (options->type != OPTION_END &&
			strcmp(negpos_name, options->name))
		{
			options++;
		}

		if (options->type != OPTION_END)
		{
			prev_pos = fprintf(stream, INDENT_STRING);
			prev_pos += fprintf(stream, "--%s", negpos_name);

			pad_usage(stream, prev_pos);
			fprintf(stream, "opposite of --no-%s\n", negpos_name);
		}
	}

	iter++;
/* END LOOP */
	}

	fputc('\n', stream);

	return PARSING_HELP;
}

static void make_cmdmode_list(struct parser_context *ctx, const struct option *iter)
{
	struct command_mode *el;

	for (; iter->type != OPTION_END; iter++)
	{
		el = ctx->cmdmode;

		if (!(iter->flags & OPTION_CMDMODE))
		{
			continue;
		}

		while (el && (el = el->next));

		el = xmalloc(sizeof(struct command_mode));

		el->valptr = iter->value;
		el->val = *(int *)iter->value;
		el->next = ctx->cmdmode;
		ctx->cmdmode = el;
	}
}

static void clean_cmdmode_list(struct parser_context *ctx)
{
	struct command_mode *prev;

	while (ctx->cmdmode)
	{
		prev = ctx->cmdmode;
		ctx->cmdmode = ctx->cmdmode->next;

		free(prev);
	}
}

int parse_options(
	int argc, const char **argv,
	const char *prefix,
	const struct option *options,
	const char *const *usages,
	enum command_parser_flag flags)
{
	struct parser_context ctx = {
		.argc0  = argc,
		.argc   = argc,
		.argv   = argv,
		.prefix = prefix,
		.flags  = flags,
		.out    = argv,
	};

	make_cmdmode_list(&ctx, options);

	while (ctx.argc)
	{
		switch (parse_option_next(&ctx, options))
		{
		case PARSING_DONE:
			break;
		case PARSING_COMPLETE:
			goto finish;
		case PARSING_NON_OPTION:
			exit(error("unknown argument ‘%s’", *ctx.argv));
		case PARSING_HELP:
			usage_with_options(usages, options, false);
			/* FALLTHRU */
		case PARSING_ERROR:
			exit(-1);
		case PARSING_UNKNOWN:
			if (ctx.argv[0][1] == '-')
			{
				error("unknown option ‘%s’", ctx.argv[0] + 2);
			}
			else if (isascii(*ctx.optstr))
			{
				error("unknown switch ‘%c’", *ctx.optstr);
			}
			else
			{
				error("unknown non-ascii option in string: "
					"‘%s’", *ctx.argv);
			}

			usage_with_options(usages, options, true);
			exit(-1);
		}

		ctx.argc--;
		ctx.argv++;
	}

finish:
	clean_cmdmode_list(&ctx);

	if (ctx.argc)
	{
		MOVE_ARRAY(ctx.out + ctx.idx, ctx.argv, ctx.argc);
	}

	ctx.out[ctx.idx + ctx.argc] = NULL;
	return ctx.idx + ctx.argc;
}
