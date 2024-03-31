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

int pk_setenv(const char *name, const char *value, int replace)
{
	if (name == NULL || value == NULL || strchr(name, '='))
	{
		return -1;
	}

	if (getenv(name) && !replace)
	{
		return 0;
	}

	size_t ll, rl;
	char *buf;

	ll = strlen(name);
	rl = strlen(value);
	buf = xmalloc(ll + rl + 2);

	memcpy(buf, name, ll);
	buf[ll++] = '=';
	memcpy(buf + ll, value, rl);
	buf[ll + rl] = 0;

	return putenv(buf);
}