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

#ifndef STRBUF_H
#define STRBUF_H

#ifdef PK_IS_DEBUG
extern unsigned sb_resize_count;
#endif

typedef struct
{
	char *data;
	size_t size;
	size_t capacity;

} stringbuffer;

/**
 * @brief Allocates and initializes a stringbuffer structure with given capacity.
 *
 * @param capacity Initial capacity of the stringbuffer's data buffer.
 * @return A pointer to the allocated stringbuffer structure, or NULL if any allocation fails.
 */
stringbuffer *sballoc(size_t capacity);

/**
 * @brief Resizes the buffer of a stringbuffer to at least a specified lower bound.
 *
 * @param strbuf The stringbuffer to resize. Must not be NULL.
 * @param lower_bound The minimum capacity required after resizing.
 * @return A pointer to the resized stringbuffer, or NULL if reallocation fails.
 *
 * @note This function increments an external counter `sb_resize_count` for debugging purposes.
 */
stringbuffer *sbresize(stringbuffer *strbuf, size_t lower_bound);

/**
 * @brief Appends a character and null-terminator to the stringbuffer, resizing if needed.
 *
 * @param strbuf The stringbuffer to append the character to. Must not be NULL.
 * @param c The character to append.
 */
void sbputc(stringbuffer *strbuf, char c);

/**
 * @brief Appends a null-terminated string to the stringbuffer, resizing if needed.
 *
 * @param strbuf The stringbuffer to append the string to. This container must already be initialized.
 * @param src The null-terminated string to append.
 */
void sbprint(stringbuffer *strbuf, const char *src);

/**
 * @brief Appends formatted data to the stringbuffer, resizing if needed.
 *
 * @param strbuf The stringbuffer to append the formatted data to. Must be initialized.
 * @param format The null-terminated format string in printf-style.
 * @param ... Variable arguments to be formatted according to `format`.
 */
void sbprintf(stringbuffer *strbuf, const char *format, ...);

/**
 * @brief Appends a formatted string with a specific length to the stringbuffer, resizing if needed.
 *
 * @param strbuf The stringbuffer to append the formatted data to. Must be initialized.
 * @param length The maximum number of characters to append, excluding the null-terminator.
 * @param format The null-terminated format string in printf-style.
 * @param ... Variable arguments to be formatted according to `format`.
 */
void sbnprintf(stringbuffer *strbuf, size_t length, const char *format, ...);

/**
 * @brief Frees the memory allocated for a stringbuffer and its internal data buffer.
 *
 * @param strbuf The stringbuffer to free. May be NULL.
 */
void sbfree(stringbuffer *strbuf);

/**
 * @brief Determines if a stringbuffer needs resizing.
 *
 * @param prev The current capacity of the stringbuffer.
 * @param next The required new size after adding more data.
 * @return true if resizing is needed (next size exceeds capacity threshold), false otherwise.
 *
 */
bool sb_need_resize(size_t prev, size_t next);

#endif /* STRBUF_H */