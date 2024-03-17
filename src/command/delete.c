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

static unsigned record_id;
static bool record_id_set;

const char *const cmd_delete_usages[] = {
	"pk delete [--record <id>] [<id>]",
	NULL,
};

const struct option cmd_delete_options[] = {
	OPTION_UNSIGNED('i', "record", &record_id, &record_id_set, "id", "id points to the record to be deleted"),
	OPTION_END(),
};

static void set_record_id(int argc, const char *arg)
{
	if (argc == 0)
	{
		exit(error("no option or argument gives a value to record id"));
	}

	if (argc > 1)
	{
		exit(error("too many arguments give value to record id"));
	}

	int errcode;

	errcode = strtou(arg, &record_id);
	errcode = process_unsigned_assignment_result(errcode, arg, "record id");

	if (errcode)
	{
		exit(errcode);
	}
}

int cmd_delete(int argc, const char **argv, UNUSED const char *prefix)
{
	argc = parse_options(argc, argv, prefix, cmd_delete_options, cmd_delete_usages, 0);
	if (!record_id_set)
	{
		set_record_id(argc, *argv);
	}

	return 0;
}