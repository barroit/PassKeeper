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

#ifndef STRUTIL_H
#define STRUTIL_H

char *mkspase(size_t length);

char *concat(const char *str1, const char *str2);

char *substr(const char *src, size_t start, size_t length);

bool is_empty_string(const char *string);

size_t u8strlen(const char *iter);

char *u8substr(const char *src, size_t start, size_t count);

int strtou(const char *str, unsigned *res);

#endif /* STRUTIL_H */