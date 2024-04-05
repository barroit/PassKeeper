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

#ifndef MESSAGE_H
#define MESSAGE_H

int error(const char *format, ...) __attribute__((format(printf, 2, 3)));
void die(const char *format, ...) __attribute__((format(printf, 2, 3), noreturn));

int error_errno(const char *format, ...) __attribute__((format(printf, 2, 3)));

void bugfl(const char *file, int line, const char *format, ...) __attribute__((format(printf, 3, 4)));
#define bug(...) bugfl(__FILE__, __LINE__, __VA_ARGS__)

#endif /* MESSAGE_H */