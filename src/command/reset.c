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

static const char *const cmd_reset_usages[] = {
	"pk reset",
	NULL,
};

int cmd_reset(int argc, const char **argv, const char *prefix)
{
	parse_options(argc, argv, prefix, &(const struct option){ 0 },
			cmd_reset_usages, PARSER_ABORT_NON_OPTION);
	return 0;
}