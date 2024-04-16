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

#define PK_CRED_DB	"PK_CRED_DB"
#define PK_CRED_KY	"PK_CRED_KY"
#define PK_EDITOR	"PK_EDITOR"
#define PK_SPINNER	"PK_SPINNER"
#define PK_RECFILE	"PK_RECFILE"

#define COMMON_RECORD_MESSAGE							\
"# Please enter the information for your password record. Lines starting \n"	\
"# with '#' will be ignored, and an empty record aborts the creation.\n"	\
"# Note: The order of each field in the record matters.\n"			\
"# Run './pk example record' to see the example."

#define PK_CRED_DB_NM	".pk-credfl"
#define PK_CRED_KY_NM	".pk-credky"

#ifdef LINUX
#define ENV_USERHOME "HOME"
#define DIRSEPSTR  "/"
#define DIRSEPCHAR '/'
#else
#define ENV_USERHOME "USERPROFILE"
#define DIRSEPSTR  "\\"
#define DIRSEPCHAR '\\'
#endif

#define FILCRT_BIT ((S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH))
#define DIRCRT_BIT ((S_IRWXU) | (S_IRWXG) | (S_IROTH | S_IXOTH))

#define UNUSED __attribute__((unused))
#define FORCEINLINE __attribute__((always_inline))

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

#if defined(LINUX) || defined(PKTEST) /* for test */
#define mkdir(path) mkdir((path), DIRCRT_BIT)
#endif

#ifdef LINUX
#define test_file_permission(p, s, m) test_file_permission_st(s, m)
#else
#define test_file_permission(p, s, m) test_file_permission_ch(p, m)
#endif

int get_bias(long *bias);

#ifdef LINUX
#define DEFAULT_EDITOR "vi"
#else
#define DEFAULT_EDITOR "notepad"
#endif

#ifdef LINUX
#define NULDEV "/dev/null"
#else
#define NULDEV "NUL"
#endif

#ifndef SIGQUIT
#define SIGQUIT 3
#endif

#endif /* COMPACT_UTIL_H */
