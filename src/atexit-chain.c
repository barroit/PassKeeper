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

#include "atexit-chain.h"

struct atexit_node
{
	atexit_func_t func;
	struct atexit_node *next;
};

static struct atexit_node *atexit_func;

void atexit_chain_push(atexit_func_t func)
{
	struct atexit_node *head;

	head = xmalloc(sizeof(struct atexit_node));

	head->func = func;
	head->next = atexit_func;

	atexit_func = head;
}

atexit_func_t atexit_chain_pop(void)
{
	if (atexit_func == NULL)
	{
		return NULL;
	}

	struct atexit_node *prev;
	atexit_func_t func;

	prev = atexit_func;
	func = atexit_func->func;

	atexit_func = atexit_func->next;

	free(prev);
	return func;
}

void apply_atexit_chain(void)
{
	atexit_func_t func;
	while ((func = atexit_chain_pop()) != NULL)
	{
		func();
	}
}
