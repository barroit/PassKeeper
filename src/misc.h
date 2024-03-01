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

// TODO: this file has a bunch of uncategorized functions. consider grouping them

#ifndef MISC_H
#define MISC_H

#define STRINGIFY(str) ((str) == ((void *)0) ? "(null)" : (*str) == '\0' ? "(empty)" : (str))

#define STRBOOL(v) ((v) ? "true" : "false")

#define MAX(x, y) ((x > y) ? x : y)

#define MIN(x, y) ((x < y) ? x : y)

#define IN_RANGE(p, x1, x2) ((p >= x1) && (p <= x2))

bool is_positive_integer(const char *str);

bool is_hexchr(char c);

#endif /* MISC_H */