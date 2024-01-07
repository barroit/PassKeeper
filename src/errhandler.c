#include "errhandler.h"
#include "rescode.h"
#include "utility.h"
#include "strbuffer.h"

#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/err.h>

extern const char *appname;

#define eprintf(format, ...) fprintf(stderr, "%s: " format "\n", appname, __VA_ARGS__)

#define eputs(str) eprintf("%s", str)

void handle_parse_cmdopts_error(int rc, const char *errmsg[2])
{
	switch (rc)
	{
		case PK_UNKNOWN_OPTION:
			eprintf("unrecognized option '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_MISSING_OPERAND:
			eprintf("option '%s' requires an operand", errmsg[ERRMSG_IK]);
			break;
		case PK_INCOMPATIBLE_TYPE:
			eprintf("incompatible value '%s' converted to option '%s'", STRINGIFY(errmsg[ERRMSG_IV]), errmsg[ERRMSG_IK]);
			break;
		default:
			abort();
	}
}

void handle_parse_cmdargs_error(int rc, const char *errmsg[3])
{
	switch (rc)
	{
		case PK_UNKNOWN_OPERATION:
			eprintf("unrecognized database operation '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_INCOMPATIBLE_TYPE:
			eprintf("incompatible value '%s' converted to option '%s'", STRINGIFY(errmsg[ERRMSG_IV]), errmsg[ERRMSG_IK]);
			break;
		case PK_UNCLEAR_OPTARG:
			eprintf("option '%s' redefined with argument value '%s'", errmsg[ERRMSG_IK], STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_TOOMANY_ARGUMENTS:
			eprintf("too many arguments ('%s'%s%s%s%s) found during parsing",
				errmsg[ERRMSG_IK],
				errmsg[ERRMSG_IV] == NULL ? "" : ", '",
				errmsg[ERRMSG_IV] == NULL ? "" : errmsg[ERRMSG_IV],
				errmsg[ERRMSG_IV] == NULL ? "" : "'",
				errmsg[ERRMSG_IE1] == NULL ? "" : ", ...");
			break;
		default:
			abort();
	}
}

void handle_missing_operation_error(void)
{
	eprintf("missing database operation\n"
		"Try '%s --help' for more information.",
		appname);
}

void handle_validate_appopt_error(int rc, const char *errmsg[3])
{
	if (is_io_error(rc))
	{
		handle_io_error(rc, errmsg);
	}
	else if (PK_UNSATISFIED_CONDITION)
	{
		eprintf("database operation '%s' requires %s%s%s",
			errmsg[ERRMSG_IK],
			errmsg[ERRMSG_IV],
			errmsg[ERRMSG_IE1] == NULL ? "" : " and ",
			errmsg[ERRMSG_IE1] == NULL ? "" : errmsg[ERRMSG_IE1]);
	}
	else
	{
		abort();
	}
}

void handle_init_database_error(int rc, const char *errmsg[2])
{
	if (is_io_error(rc))
	{
		handle_io_error(rc, errmsg);
	}
	else
	{
		handle_sqlite_error(rc);
	}
}

void handle_sqlite_error(int rc)
{
	const char *errstr;
	if (strcmp((errstr = sqlite3_errstr(rc)), "unknown error") == 0)
	{
		abort(); /* You definitely forgot to handle error */
	}

	eputs(errstr);
}

void handle_io_error(int rc, const char *errmsg[2])
{
	switch (rc)
	{
		case PK_MISSING_FILE:
			eprintf("no such file or directory '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_FILE_EXISTS:
			eprintf("file already exists '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_PERMISSION_DENIED:
			eprintf("interaction with '%s' is not allowed", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_INVALID_PATHNAME:
			eprintf("unsupported pathname '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_MKDIR_FAILURE:
			eprintf("cannot create directory for file '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		default:
			abort();
	}
}

void handle_db_key_error(int rc, const char *errmsg[2])
{
	if (is_io_error(rc))
	{
		handle_io_error(rc, errmsg);
		return;
	}

	switch (rc)
	{
		case PK_INVALID_KEY:
			eprintf("invalid key read from file '%s'", errmsg[ERRMSG_IV]);
			break;
		case PK_INCORRECT_KEY:
			eputs("database key is incorrect");
			break;
		case PK_KEYGEN_FAILURE:
			eputs(ERR_reason_error_string(ERR_get_error()));
			break;
		default:
			abort();
	}
}

bool is_io_error(int rc)
{
	return IN_RANGE(rc, 30, 49);
}