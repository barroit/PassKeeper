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

size_t levenshtein_w(
	const char *s, const char *t,
	int iw, int dw, int sw, int tw)
{
	int *v9, *v0, *v1, *v39;
	size_t m, n;
	size_t i, ii, ret;

	m = strlen(s);
	n = strlen(t);

	MALLOC_ARRAY(v9, n + 1);
	MALLOC_ARRAY(v0, n + 1);
	MALLOC_ARRAY(v1, n + 1);

	array_for_each_idx(i, n)
	{
		v0[i] = i * iw;
	}

	array_for_each(i, m)
	{
/* START LOOP */
	v1[0] = ( i + 1 ) * dw;

	array_for_each(ii, n)
	{
		/* substitution */
		if (s[i] == t[ii])
		{
			v1[ii + 1] = v0[ii];
		}
		else
		{
			v1[ii + 1] = v0[ii] + sw;
		}

		/* transposition */
		if (i > 0 && ii > 0 &&
		     s[i - 1] == t[ii] && s[i] == t[ii - 1] &&
		      v1[ii + 1] > v9[ii - 1] + tw)
		{
			v1[ii + 1] = v9[ii - 1] + tw;
		}

		/* deletion */
		if (v1[ii + 1] > v0[ii + 1] + dw)
		{
			v1[ii + 1] = v0[ii + 1] + dw;
		}

		/* insertion */
		if (v1[ii + 1] > v1[ii] + iw)
		{
			v1[ii + 1] = v1[ii] + iw;
		}
	}

	v39 = v9;
	v9 = v0;
	v0 = v1;
	v1 = v39;
/* END LOOP */
	}

	ret = v0[n];
	free(v9);
	free(v0);
	free(v1);

	return ret;
}
