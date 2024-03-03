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

#include "rcque.h"
#include "debug.h"
#include "rescode.h"

#ifdef PK_IS_DEBUG
size_t rcque_size = 0;
#endif

recordqueue *rcqalloc(void)
{
	recordqueue *rcque;
	if ((rcque = malloc(sizeof(recordqueue))) == NULL)
	{
		return NULL;
	}

	rcque->front = NULL;
	rcque->back = NULL;

	return rcque;
}

recordfield *rcfalloc(void)
{
	recordfield *rcfld;
	if ((rcfld = malloc(sizeof(recordfield))) == NULL)
	{
		return NULL;
	}

	rcfld->id = NULL;
	rcfld->sitename = NULL;
	rcfld->siteurl = NULL;
	rcfld->username = NULL;
	rcfld->password = NULL;
	rcfld->authtext = NULL;
	rcfld->bakcode = NULL;
	rcfld->comment = NULL;
	rcfld->sqltime = NULL;
	rcfld->modtime = NULL;

	rcfld->sitename_length = 0;
	rcfld->username_length = 0;
	rcfld->password_length = 0;

	return rcfld;
}

recordfield *enrcque(recordqueue *rcque, recordfield *data)
{
	recordnode *rcnd;
	if ((rcnd = malloc(sizeof(recordnode))) == NULL)
	{
		return NULL;
	}

	rcnd->data = data;
	rcnd->next = NULL;

	// <-----|
	//     front ----------> back
	//                        |-----<
	if (rcque->back == NULL)
	{
		rcque->front = rcnd;
	}
	else
	{
		rcque->back->next = rcnd;
	}
	rcque->back = rcnd;

	debug_execute(rcque_size++);

	return data;
}

recordfield *dercque(recordqueue *rcque)
{
	if (rcque->front == NULL)
	{
		return NULL;
	}

	recordnode *tmp = rcque->front;
	recordfield *data = tmp->data;

	if ((rcque->front = rcque->front->next) == NULL)
	{
		rcque->back = NULL;
	}

	free(tmp);

	debug_execute(rcque_size--);

	return data;
}

void rcffree(recordfield *data)
{
	if (data == NULL)
	{
		return;
	}

	free(data->id);
	free(data->sitename);
	free(data->siteurl);
	free(data->username);
	free(data->password);
	free(data->authtext);
	free(data->bakcode);
	free(data->comment);
	free(data->sqltime);
	free(data->modtime);
	free(data);
}

void rcqfree(recordqueue *rcque)
{
	if (rcque == NULL)
	{
		return;
	}

	recordfield *data;
	while ((data = dercque(rcque)) != NULL)
	{
		rcffree(data);
	}

	free(rcque);
}