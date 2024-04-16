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
 * The setenv() function shall update or add a variable in the environment of
 * the calling process. The envname argument points to a string containing
 * the name of an environment variable to be added or altered. The
 * environment variable shall be set to the value to which envval points. The
 * function shall fail if envname points to a string which contains an '='
 * character. If the environment variable named by envname already exists and
 * the value of overwrite is non-zero, the function shall return success and
 * the environment shall be updated. If the environment variable named by
 * envname already exists and the value of overwrite is zero, the function
 * shall return success and the environment shall remain unchanged.
 * 
 * Upon successful completion, zero shall be returned. Otherwise, -1 shall be
 * returned, errno set to indicate the error, and the environment shall be
 * unchanged.
 * 
 * The setenv() function shall fail if:
 * [EINVAL]
 *     The envname argument points to an empty string or points to a string
 *     containing an '=' character.
 * [ENOMEM]
 *     Insufficient memory was available to add a variable or its value to
 *     the environment.
 */
int pk_setenv(const char *envname, const char *envval, int overwrite)
{
	if (envname == NULL || *envname == 0 || strchr(envname, '='))
	{
		errno = EINVAL;
		return -1;
	}

	if (getenv(envname) && !overwrite)
	{
		return 0;
	}

	size_t envname_len, envval_len;
	char *buf;

	envname_len = strlen(envname);
	envval_len = strlen(envval);
	if ((buf = malloc(envname_len + envval_len + 2)) == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	memcpy(buf, envname, envname_len);
	buf[envname_len] = '=';
	memcpy(buf + envname_len + 1, envval, envval_len);
	buf[envname_len + 1 + envval_len] = 0;

	putenv(buf);
	return 0;
}
