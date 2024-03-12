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

static bool show_build_options;

static const char *usages[] = {
	"pk count [--search <pattern>]",
	NULL,
};


int cmd_version(int argc, const char **argv)
{
	struct option cmd_version_option[] = {
		OPTION_BOOL(0, "build-options", &show_build_options, "also print build information"),
		OPTION_END(),
	};

	argc = parse_option(argc, argv, cmd_version_option, usages, 0);
	
	if (show_build_options)
	{
		// ...
	}

	printf_ln("pk version %s.%s", PROJECT_VERSION, GIT_COMMIT_HASH);

	return 0;
}