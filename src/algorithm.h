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

#ifndef ALGORITHM_H
#define ALGORITHM_H

size_t levenshtein_w(const char *s, const char *t, int iw, int dw, int sw, int tw);

#define levenshtein(s, t)\
	levenshtein_w(s, t, 1, 1, 1, 1)

void merge_sort(void *array, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

#define MSORT(array, nmemb, compar)\
	merge_sort(array, nmemb, sizeof(*(array)), compar)

#endif /* ALGORITHM_H */
