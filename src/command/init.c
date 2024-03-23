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

#include "parse-options.h"
#include "filesys.h"
#include "rawnumop.h"
#include "credky.h"
#include "strlist.h"

static struct
{
	bool store_key;
	const char *key;
} user;

static struct cipher_config cc = {
	.kdf_iter = CIPHER_DEFAULT_KDF_ITER,
	.page_size = CIPHER_DEFAULT_PAGE_SIZE,
	.compatibility = CIPHER_DEFAULT_COMPATIBILITY,
};

const char *const cmd_init_usages[] = {
	"pk init [--encrypt[=<pwd>]] [<options>]",
	NULL,
};

const struct option cmd_init_options[] = {
	OPTION_OPTARG(0, "encrypt", &user.key, 1, "pwd", "password used to encryption"),
	OPTION_BOOLEAN(0, "remember", &user.store_key, "store password"),
	OPTION_GROUP(""),
	OPTION_STRING(0, "kdf-algorithm", &cc.kdf_algorithm, "KDF algorithm used to generate encryption key for database"),
	OPTION_STRING(0, "hmac-algorithm", &cc.hmac_algorithm, "HMAC algorithm used to detect illegal data tampering"),
	OPTION_UNSIGNED(0, "compatibility", &cc.compatibility, "version of api to used"),
	OPTION_UNSIGNED(0, "page-size", &cc.page_size, "size of a page"),
	OPTION_UNSIGNED(0, "kdf-iter", &cc.kdf_iter, "key derivation iteration times"),
	OPTION_END(),
};

static void prepare_file_path(char pathbuf[], const char *prefix, const char *envname, const char *defname)
{
	const char *envpath;
	char *pfpath;

	if ((envpath = getenv(envname)) == NULL)
	{
		pfpath = prefix_filename(prefix, defname);
		strcpy(pathbuf, pfpath);
		free(pfpath);
	}
	else
	{
		strcpy(pathbuf, envpath);
	}

	if (access(pathbuf, F_OK) == 0)
	{
		die("file '%s' already exists", pathbuf);
	}

	prepare_file_directory(pathbuf);
}

#define BINKEY_LENGTH		32
#define HEXKEY_LENGTH		64
#define SALT_LENGTH		32
#define HEXKEYSTR_LENGTH	HEXKEY_LENGTH + 3
#define SALT_HEXKEYSTR_LENGTH	HEXKEY_LENGTH + SALT_LENGTH + 3
#define BLOBKEY_MAX		100 /* contains null-terminator */

static void verify_raw_key_data(const char *key, size_t *outsz)
{
	*outsz = strlen(key);

	if (*outsz != HEXKEYSTR_LENGTH && *outsz != SALT_HEXKEYSTR_LENGTH)
	{
		exit(error("invalid key length \"%s\"", key));
	}
	else if (!is_hexstr(key + 2, HEXKEY_LENGTH))
	{
		exit(error("key contains invalid char \"%s\"", key));
	}
	else if (*outsz == SALT_HEXKEYSTR_LENGTH &&
		!is_saltstr(key + 2 + HEXKEY_LENGTH, SALT_LENGTH))
	{
		exit(error("invalid key salt \"%s\"", key));
	}
}

static const char *kdf_algorithms[] = {
	CIPHER_DEFAULT_KDF_ALGORITHM,
	"PBKDF2HMAC_SHA256",
	"PBKDF2_HMAC_SHA1",
	NULL,
};

static const char *hmac_algorithms[] = {
	CIPHER_DEFAULT_HMAC_ALGORITHM,
	"HMAC_SHA256",
	"HMAC_SHA1",
	NULL,
};

static int verify_algorithm(const char *name, const char *arr[], const char *defalgo)
{
	if (!in_string_array(arr, name))
	{
		exit(error("algorithm '%s' not found", name));
	}

	if (!strcmp(name, defalgo))
	{
		return 1;
	}

	return 0;
}

int cmd_init(UNUSED int argc, const char **argv, const char *prefix)
{
	char db_path[PATH_MAX], ky_path[PATH_MAX];

	char hexkey_str[BLOBKEY_MAX];
	size_t hexkey_str_len;

	bool encrypt_db, use_binkey, use_pkgen_binkey, use_passphrase, use_key_file;

	parse_options(argc, argv, prefix, cmd_init_options, cmd_init_usages, PARSER_ABORT_NON_OPTION);

	encrypt_db = !!user.key;
	if (user.store_key && !encrypt_db)
	{
		exit(error("remember option must be used together with encrypt option"));
	}

	use_pkgen_binkey = (intptr_t)user.key == 1;
	use_binkey = encrypt_db && !use_pkgen_binkey && *user.key == 'x';
	use_passphrase = encrypt_db && !use_binkey;

	use_key_file = 0;

	/* check kdf algorithm */
	if (cc.kdf_algorithm)
	{
		use_key_file |= verify_algorithm(cc.kdf_algorithm, kdf_algorithms, CIPHER_DEFAULT_KDF_ALGORITHM);
	}

	/* check hmac algorithm */
	if (cc.hmac_algorithm)
	{
		use_key_file |= verify_algorithm(cc.hmac_algorithm, hmac_algorithms, CIPHER_DEFAULT_HMAC_ALGORITHM);
	}

	/* check key iter */
	use_key_file |= cc.kdf_iter != CIPHER_DEFAULT_KDF_ITER;

	/* check page size */
	if (!in_range_u(cc.page_size, 512, 65536, 1) || !is_pow2(cc.page_size))
	{
		exit(error("invalid page size '%d'", cc.page_size));
	}
	else
	{
		use_key_file |= cc.page_size != CIPHER_DEFAULT_PAGE_SIZE;
	}

	/* check compatibility */
	if (!in_range_u(cc.compatibility, CIPHER_MIN_COMPATIBILITY, CIPHER_MAX_COMPATIBILITY, 1))
	{
		exit(error("unknown compatibility '%d'", cc.compatibility));
	}
	else
	{
		use_key_file |= cc.compatibility != CIPHER_DEFAULT_COMPATIBILITY;
	}

	/* check if store key */
	use_key_file |= use_binkey || user.store_key;

	prepare_file_path(db_path, prefix, PK_CRED_DB, PK_CRED_DB_NM);

	if (use_key_file)
	{
		prepare_file_path(ky_path, prefix, PK_CRED_KY, PK_CRED_KY_NM);
	}

	if (!encrypt_db)
	{
		goto setup_key_file;
	}

	if (use_pkgen_binkey)
	{
		char *hexkey_str_tmp;

		cc.key = random_bytes(BINKEY_LENGTH);
		cc.keylen = BINKEY_LENGTH;

		hexkey_str_tmp = bin2hex(xmemdup(cc.key, cc.keylen), cc.keylen);
		hexkey_str_len = snprintf(hexkey_str, HEXKEYSTR_LENGTH, "x'%s'", hexkey_str_tmp);
		free(hexkey_str_tmp);
	}
	else if (use_binkey)
	{
		size_t hexkey_len;

		verify_raw_key_data(user.key, &hexkey_str_len);

		/* -2 skip prefixing x' and -1 skip tailing ' */
		hexkey_len = hexkey_str_len - 2 - 1;
		cc.key = hex2bin(strdup(user.key + 2), hexkey_len);
		cc.keylen = hexkey_len / 2;

		strncpy(hexkey_str, user.key, sizeof(hexkey_str));
	}
	else
	{
		cc.keylen = strlen(user.key);
		if (cc.keylen == 0)
		{
			exit(error("passphrase key is empty"));
		}

		cc.key = xmemdup(user.key, cc.keylen);
	}

	cc.is_binary_key = !use_passphrase;

setup_key_file:

	if (!use_key_file)
	{
		goto init_database;
	}

	uint8_t *cc_buf;
	size_t cc_buflen;

	cc_buf = serialize_cipher_config(&cc, &cc_buflen);

	free(cc_buf);

init_database:

	printf("%d\n", use_key_file);

	// char hexkey_str[BLOBKEY_MAX];
	// size_t hexkey_str_len;

	// uint8_t *binkey;
	// size_t binkey_len;

	return 0;
}