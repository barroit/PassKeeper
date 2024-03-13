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

const char *const cmd_version_usages[] = {
	"pk version [--build-options]",
	NULL,
};

const struct option cmd_version_options[] = {
	OPTION_BOOL(0, "build-options", &show_build_options, "also print build information"),
	OPTION_END(),
};

int cmd_version(int argc, const char **argv)
{
	argc = parse_option(argc, argv, cmd_version_options, cmd_version_usages, 0);

	struct strbuf sb = STRBUF_INIT;
	strbuf_print(&sb, "pk version "PROJECT_VERSION);

	if (show_build_options)
	{
		strbuf_putc(&sb, '\n');
		strbuf_puts(&sb, "architecture: "ARCHITECTURE);
		strbuf_puts(&sb, "built from commit: "BUILD_COMMIT);
	}
	else
	{
		strbuf_printf(&sb, ".%.11s\n", BUILD_COMMIT);
	}

	strbuf_printf(&sb, "written by %s <%s>", AUTHOR, CONTACT);

	puts(sb.buf);
	strbuf_destroy(&sb);

	return 0;
}