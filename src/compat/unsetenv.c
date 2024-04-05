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

/**
 * The unsetenv() function shall remove an environment variable from the
 * environment of the calling process. The name argument points to a string,
 * which is the name of the variable to be removed. The named argument shall
 * not contain an '=' character. If the named variable does not exist in the
 * current environment, the environment shall be unchanged and the function
 * is considered to have completed successfully.
 * 
 * Upon successful completion, zero shall be returned. Otherwise, -1 shall be
 * returned, errno set to indicate the error, and the environment shall be
 * unchanged.
 * 
 * The unsetenv() function shall fail if:
 * [EINVAL]
 *     The name argument points to an empty string, or points to a string
 *     containing an '=' character.
 */
int pk_unsetenv(const char *name)
{
	if (*name == 0 || strchr(name, '='))
	{
		errno = EINVAL;
		return -1;
	}

	putenv(strdup(name));
	return 0;
}