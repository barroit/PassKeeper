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

#ifdef test_file_mode
#undef test_file_mode
#endif

/**
 * if mode `m1__` has flag `f1__` then check if
 * mode `m2__` has flag `f2__`, else true
 */
#define MATCH_MODE(m1__, f1__, m2__, f2__)\
	( ( (m1__) & (f1__) ) ? ( (m2__) & (f2__) ) : 1 )

int test_file_mode(struct stat *st, int mode)
{
	if (st->st_uid == geteuid()) /* euid matches owner id? */
	{
		if (MATCH_MODE(mode, R_OK, st->st_mode, S_IRUSR) &&
		     MATCH_MODE(mode, W_OK, st->st_mode, S_IWUSR) &&
		      MATCH_MODE(mode, X_OK, st->st_mode, S_IXUSR))
		{
			return 0;
		}
	}
	else if (st->st_gid == getegid()) /* egid matches group id? */
	{
		if (MATCH_MODE(mode, R_OK, st->st_mode, S_IRGRP) &&
		     MATCH_MODE(mode, W_OK, st->st_mode, S_IWGRP) &&
		      MATCH_MODE(mode, X_OK, st->st_mode, S_IXGRP))
		{
			return 0;
		}
	}
	else /* check other bits */
	{
		if (MATCH_MODE(mode, R_OK, st->st_mode, S_IROTH) &&
		     MATCH_MODE(mode, W_OK, st->st_mode, S_IWOTH) &&
		      MATCH_MODE(mode, X_OK, st->st_mode, S_IXOTH))
		{
			return 0;
		}
	}

	return 1;
}
