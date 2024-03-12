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

bool is_command(const char *cmdname);

int cmd_count(int argc, const char **argv);
int cmd_create(int argc, const char **argv);
int cmd_delete(int argc, const char **argv);
int cmd_help(int argc, const char **argv);
int cmd_init(int argc, const char **argv);
int cmd_read(int argc, const char **argv);
int cmd_update(int argc, const char **argv);
int cmd_version(int argc, const char **argv);

#endif /* COMMAND_H */