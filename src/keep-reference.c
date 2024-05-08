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

struct leakref
{
	struct leakref *next;
	uint8_t data[FLEX_ARRAY];
};

static struct leakref *suppressed_leaks;

void keep_reference(const void *ptr, size_t size)
{
	struct leakref *head;

	FLEX_ALLOC_ARRAY(head, data, ptr, size);

	head->next = suppressed_leaks;
	suppressed_leaks = head;
}
