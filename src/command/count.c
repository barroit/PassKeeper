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

static const char *search_pattern = "%";

static const char *usagestr[] = {
	"pk count [--search <pattern>]",
	NULL,
};

int cmd_count(int argc, const char **argv)
{
	struct option cmd_count_option[] = {
		OPTION_STRING(0, "search", &search_pattern, "pattern", "count record for a particular site"),
		OPTION_END(),
	};

	argc = parse_option(argc, argv, cmd_count_option, usagestr, PARSER_STOP_AT_NON_OPTION);
	printf("s: %s, a: %d\n", search_pattern, argc);

	return 0;
}