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
#include <sys/types.h>
#include <assert.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <sqlite3.h>
#include <signal.h>
#include <fcntl.h>

#ifdef LINUX
#include <time.h>
#else
#include <timezoneapi.h>
#endif

#ifdef WINDOWS_NATIVE
#include <windef.h>
#include <winbase.h>
#include <io.h>
#include <stringapiset.h>
#include <handleapi.h>
#include <errhandlingapi.h>
#include <processthreadsapi.h>
#endif

#ifdef LINUX
#include <sys/wait.h>
#endif

#ifndef PK_CRED_DB
#define PK_CRED_DB "PK_CRED_DB"
#endif

#ifndef PK_CRED_CC
#define PK_CRED_CC "PK_CRED_CC"
#endif

#ifndef PK_TMP_REC
#define PK_TMP_REC "PK_TMP_REC"
#endif

#ifndef PK_EDITOR
#define PK_EDITOR "PK_EDITOR"
#endif

#ifndef PK_SPINNER
#define PK_SPINNER "PK_SPINNER"
#endif

#ifndef COMMON_RECORD_MESSAGE
#define COMMON_RECORD_MESSAGE							\
"# Please enter the information for your password record. Lines starting\n"	\
"# with '#' will be ignored, and an empty record aborts the creation.\n"	\
"# Certain symbols prefixing a line (such as '#' and ':') have special\n"	\
"# meanings for PassKeeper. To include such symbols in the text without\n"	\
"# invoking their special meanings, start the line with '|'.\n"			\
"# Run './pk example record' to see the example."
#endif

#ifndef PK_CRED_DB_DEFPATH
#define PK_CRED_DB_DEFPATH  ".pk-cred-db"
#endif

#ifndef PK_CRED_CC_DEFPATH
#define PK_CRED_CC_DEFPATH  ".pk-cred-cc"
#endif

#ifndef PK_TMP_REC_DEFPATH
#define PK_TMP_REC_DEFPATH  ".pk-tmp-rec"
#endif

#ifdef LINUX
#define ENV_USERHOME  "HOME"
#define DIRSEPSTR     "/"
#define DIRSEPCHAR    '/'
#else
#define ENV_USERHOME  "USERPROFILE"
#define DIRSEPSTR     "\\"
#define DIRSEPCHAR    '\\'
#endif

#define FILCRT_BIT ((S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH))
#define DIRCRT_BIT ((S_IRWXU) | (S_IRWXG) | (S_IROTH | S_IXOTH))

#ifdef NO_ALWAYS_INLINE
#define FORCEINLINE
#else
#define FORCEINLINE __attribute__((always_inline))
#endif

#ifdef NO_UNUSED
#define UNUSED
#else
#define UNUSED __attribute__((unused))
#endif

#ifdef NO_STRCHRNUL
char *pk_strchrnul(const char *s, int c);
#define strchrnul pk_strchrnul
#endif

#ifdef NO_SETENV
int pk_setenv(const char *name, const char *value, int replace)
	__attribute__((nonnull(2)));
#define setenv pk_setenv
#endif

#ifdef NO_UNSETENV
int pk_unsetenv(const char *name) __attribute__((nonnull(1)));
#define unsetenv pk_unsetenv
#endif

#ifdef NO_DIRNAME
char *pk_dirname(char *path);
#define dirname pk_dirname
#else
#include <libgen.h>
#endif

#ifdef NO_STRDUP
char *pk_strdup(const char *s);
#define strdup pk_strdup
#endif

#ifdef NO_STRNDUP
char *pk_strndup(const char *s, size_t size);
#define strndup pk_strndup
#endif

#if defined(LINUX) || defined(PKTEST) /* for test */
#define mkdir(path) mkdir((path), DIRCRT_BIT)
#endif

#ifdef LINUX
int test_file_permission_st(struct stat *st, int mode);

#define test_file_permission(f, s, m) test_file_permission_st(s, m)
#else
#define test_file_permission(f, s, m) (access(f, m) == -1)
#endif

#ifdef LINUX
#define is_dumb_term (getenv("TERM") == NULL)
#else
#define is_dumb_term (false)
#endif

#ifdef LINUX
#define DEFAULT_EXT_EDITOR "vi"
#else
#define DEFAULT_EXT_EDITOR "notepad"
#endif

#ifdef LINUX
#define NULDEV  "/dev/null"
#else
#define NULDEV  "NUL"
#endif

#ifndef SIGQUIT
#define SIGQUIT 3
#endif

#ifdef __STDC_LIB_EXT1__
#define zeromem(p__, l__) memset_s((p__), (l__), 0, (l__))
#else
void pk_zeromem(void *ptr, size_t len);

#define zeromem pk_zeromem
#endif

#ifdef NO_GETLINE
ssize_t pk_getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);

#define getline pk_getline
#endif

#ifdef LINUX
#include <termios.h>
#else
struct termios
{
	stdin_handle;
	console_mode;
}
#endif

#endif /* COMPACT_UTIL_H */
