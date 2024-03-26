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

#ifndef COMPACT_UTIL_H
#define COMPACT_UTIL_H

#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <inttypes.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <assert.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <sqlite3.h>

#ifdef POSIX
#define ENV_USERHOME "HOME"
#define DIRSEPSTR  "/"
#define DIRSEPCHAR '/'
#else
#define ENV_USERHOME "USERPROFILE"
#define DIRSEPSTR  "\\"
#define DIRSEPCHAR '\\'
#endif

#define UNUSED __attribute__((unused))

// #define POSIX_ONLY

// #ifndef POSIX
// #define POSIX_ONLY static
// #endif

#ifdef NO_STRCHRNUL
char *pk_strchrnul(const char *s, int c);
#define strchrnul pk_strchrnul
#endif

#ifdef NO_SETENV
int pk_setenv(const char *name, const char *value, int replace);
#define setenv pk_setenv
#endif

#ifdef POSIX
#include <libgen.h>
#endif

#ifdef NO_DIRNAME
char *pk_dirname(char *path);
#define dirname pk_dirname
#endif

#if defined(POSIX) || defined(PKTEST) /* for test */
#define mkdir(path) mkdir((path), 0775)
#endif

#ifdef POSIX
#define test_file_permission(p, s, m) test_file_permission_st(s, m)
#else
#define test_file_permission(p, s, m) test_file_permission_ch(p, m)
#endif

#ifdef POSIX
#include <time.h>
#else
#include <timezoneapi.h>
#endif

int get_bias(long *bias);

#endif /* COMPACT_UTIL_H */