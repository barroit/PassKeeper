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

#ifdef strerror
#undef strerror
#endif

#define EBUF_SIZE 256

static void strcpy_errmsg(char *buf, const char *msg)
{
	size_t len;

	if ((len = strlen(msg) >= EBUF_SIZE))
	{
		bug("error message length(‘%"PRIuMAX"’) is more than %d "
			"characters", len, EBUF_SIZE);
	}

	memcpy(buf, msg, len + 1);
}

char *pk_strerror(int errnum)
{
	static char buf[EBUF_SIZE];

	if (errnum >= 0)
	{
		strcpy_errmsg(buf, strerror(errnum));

		if (isupper(*buf))
		{
			buf[0] = tolower(*buf);
		}

		return buf;
	}

	switch (errnum)
	{
	case ENOTREG:
		strcpy_errmsg(buf, "not a regular file");
		break;
	default:
		bug("unknown errnum: ‘%d’", errnum);
	}

	return buf;
}
