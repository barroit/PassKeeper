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

#ifndef STRUTIL_H
#define STRUTIL_H

/**
 * @brief Creates a string filled with spaces, null-terminated.
 *
 * @param length The number of spaces in the string.
 * @return A pointer to the allocated space-filled string, or NULL if allocation fails.
 *
 * @note Result string must be freed by caller.
 */
char *mkspase(size_t length);

/**
 * @brief Concatenates two null-terminated strings into a new, null-terminated string.
 *
 * @param str1 First string to concatenate. Must not be NULL.
 * @param str2 Second string to concatenate. Must not be NULL.
 * @return A pointer to the concatenated string, or NULL on failure.
 *
 * @note Returns NULL if `str1` or `str2` is NULL. Result string must be freed by caller.
 */
char *concat(const char *str1, const char *str2);

/**
 * @brief Extracts a substring from a specified position.
 * 
 * @param src Source string.
 * @param start Index to start at.
 * @param length Chars to copy; 0 for full copy from `start`.
 * @return New string with specified substring, null-terminated, or NULL on failure.
 * 
 * @warning Undefined behavior if `start` or `start + length` exceeds `src` length.
 * @note Returns NULL if `src` is NULL. Result string must be freed by caller.
 */
char *substr(const char *src, size_t start, size_t length);

bool is_empty_string(const char *string);

#endif /* STRUTIL_H */