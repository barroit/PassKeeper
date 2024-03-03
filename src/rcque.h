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

#ifndef RCQUE_H
#define RCQUE_H

#ifdef PK_IS_DEBUG
extern size_t rcque_size;
#endif

typedef struct
{
	char *id;
	char *sitename;
	char *siteurl;
	char *username;
	char *password;
	char *authtext;
	char *bakcode;
	char *comment;
	char *sqltime;
	char *modtime;

	// TODO: change type to size_t
	int sitename_length;
	int username_length;
	int password_length;

} recordfield;

typedef struct node
{
	recordfield *data;
	struct node *next;

} recordnode;

typedef struct
{
	recordnode *front;
	recordnode *back;

} recordqueue;

/**
 * @brief Allocates and initializes a new record queue.
 *
 * @return A pointer to the newly allocated and initialized `recordqueue`.
 *
 * @note Returns NULL if the memory allocation fails.
 */
recordqueue *rcqalloc(void);

/**
 * @brief Allocates and initializes a new record field.
 *
 * @return A pointer to the newly allocated and initialized `recordfield`.
 *
 * @note  Returns NULL if the memory allocation fails.
 */
recordfield *rcfalloc(void);

/**
 * @brief Enqueues a record field into a record queue.
 *
 * @param rcque Pointer to the record queue where the data will be enqueued.
 * @param data Pointer to the `recordfield` data to be enqueued.
 * @return Returns the provided `data` pointer on success.
 *
 * @note Returns NULL if memory allocation for the new `recordnode` fails.
 */
recordfield *enrcque(recordqueue *rcque, recordfield *data);

/**
 * @brief Dequeues a record field from a record queue.
 *
 * @param rcque Pointer to the record queue from which the data will be dequeued.
 * @return The `recordfield` data that was at the front of the queue.
 *
 * @note Returns NULL if the queue is empty or if the `rcque` is NULL.
 */
recordfield *dercque(recordqueue *rcque);

/**
 * @brief Frees a record field and its associated resources.
 *
 * @param data Pointer to the `recordfield` structure to be freed.
 */
void rcffree(recordfield *data);

/**
 * @brief Frees a record queue and all its contained record fields.
 * 
 * @param rcque Pointer to the `recordqueue` to be freed.
 */
void rcqfree(recordqueue *rcque);

#endif /* RCQUE_H */