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

int warn(const char *warning, ...) __attribute__((format(printf, 2, 3)));
int error(const char *err, ...) __attribute__((format(printf, 2, 3)));
void die(const char *reason, ...) __attribute__((format(printf, 2, 3)));

void bug_fl(const char *file, int line, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
#define bug(...) bug_fl(__FILE__, __LINE__, __VA_ARGS__)

void run_default_spinner(FILE *stream, useconds_t period);

void run_kawaii_spinner(FILE *stream, useconds_t period);

#endif /* MESSAGE_H */