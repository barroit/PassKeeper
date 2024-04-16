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

enum file_test_result
{
	F_NOT_ALLOW = 1,
	F_NOT_FILE,
	F_NOT_DIR,
	F_NOT_EXISTS,
};

static inline const char *get_user_home(void)
{
	const char *home;
	if ((home = getenv(ENV_USERHOME)) == NULL)
	{
		die("your user home corrupted in env");
	}
	return home;
}

/**
 * append `filename` to `prefix` if needed
 */
char *prefix_filename(const char *prefix, const char *filename);

void prepare_file_directory(const char *pathname);

#endif /* FILESYS_H */
