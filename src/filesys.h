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

static inline bool exists(const char *pathname)
{
	return pathname != NULL && !access(pathname, F_OK);
}

static inline bool file_avail(const char *pathname)
{
	return pathname != NULL && !access(pathname, F_OK | R_OK | W_OK);
}

static inline bool dir_avail(const char *dirname)
{
	return dirname != NULL && !access(dirname, F_OK | R_OK | W_OK | X_OK);
}

char *prefix_filename(const char *prefix, const char *filename);

#endif /* FILESYS_H */