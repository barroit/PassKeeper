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
 * @deprecated use `u8strlen`
 * 
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

// TODO: fill docs
bool is_empty_string(const char *string);

/**
 * @brief Counts the number of UTF-8 encoded characters in a string.
 * 
 * @param iter Pointer to the null-terminated UTF-8 encoded string.
 * @return The number of UTF-8 encoded characters in the string.
 */
size_t u8strlen(const char *iter);

/**
 * @brief Extracts a substring from a UTF-8 string starting at a given character index.
 * 
 * @param iter The source UTF-8 encoded string from which to extract the substring.
 * @param start_index The zero-based index of the first character to include in the substring.
 * @param substr_lenght The number of characters to include in the substring; if 0, extracts until the end of the string.
 * @return A newly allocated string containing the specified substring, null-terminated. Returns NULL on failure or if `iter` is NULL.
 * 
 * @note The caller is responsible for freeing the returned string.
 */
char *u8substr(const char *src, size_t start, size_t count);

#endif /* STRUTIL_H */