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

#ifndef COMMAND_H
#define COMMAND_H

enum cmdreq
{
	USE_CREDDB  = 1 << 0,
	USE_RECFILE = 1 << 1,
};

struct cmdinfo
{
	const char *name;
	int (*handle)(int argc, const char **argv, const char *prefix);
	enum cmdreq reqs;
};

const struct cmdinfo *find_command(const char *cmd);

char **get_approximate_command(const char *cmd);

#endif /* COMMAND_H */
