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

#ifndef FILESYS_H
#define FILESYS_H

void get_working_dir_routine(const char **out, bool force);

/**
 * get the current working directory, this function
 * uses internal buffer to cache the result
 */
#define get_working_dir(out) get_working_dir_routine(out, false)

/**
 * get the current working directory, this function
 * refresh the cache
 */
#define force_get_working_dir(out) get_working_dir_routine(out, true)

#ifdef LINUX
#define is_abs_path(p__) (*(p__) && *(p__) == '/')
#else
#define is_abs_path(p__)\
	*(p__) && in_range_i(*(p__), 'A', 'Z') && (p__)[1] == ':'
#endif

/**
 * append `filename` to `prefix` if needed
 */
char *prefix_filename(const char *prefix, const char *filename);

int make_file_dir_avail(const char *filepath);

void populate_file(const char *name, const char *buf, size_t buflen);

int access_regfile(const char *name, int type);

#endif /* FILESYS_H */
