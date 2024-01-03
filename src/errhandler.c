#include "errhandler.h"
#include "rescode.h"
#include "utility.h"
#include "strbuffer.h"

#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

void handle_validate_appopt_error(int rc, const char *errmsg[3])
{
	switch (rc)
	{
		case PK_FILE_INACCESSIBLE:
			eprintf("inaccessed database pathname '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_UNSATISFIED_CONDITION:
			eprintf("database operation '%s' requires %s%s%s",
				errmsg[ERRMSG_IK],
				errmsg[ERRMSG_IV],
				errmsg[ERRMSG_IE1] == NULL ? "" : " and ",
				errmsg[ERRMSG_IE1] == NULL ? "" : errmsg[ERRMSG_IE1]);
			break;
		default:
			abort();
	}
}

void handle_init_database_error(int rc, const char *errmsg[2])
{
	switch (rc)
	{
		case PK_INVALID_PATHNAME:
			eprintf("unsupported pathname '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_FILE_EXIST:
			eprintf("db file already exists '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_MKDIR_FAILURE:
			eprintf("Unable to create directory for file '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		default:
			handle_sqlite_error(rc);
	}
}

void handle_apply_key_error(int rc, const char *errmsg[2])
{
	switch (rc)
	{
		case PK_FILE_INACCESSIBLE:
			eprintf("Unable to read file '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_INVALID_KEY:
			eprintf("invalid key found in '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_MKDIR_FAILURE:
			eprintf("Unable to create directory for file '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_MKFILE_FAILURE:
			eprintf("Unable to create file with path '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		default:
			abort();
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

void handle_missing_operation_error(void)
{
	eprintf("missing database operation\n"
		"Try '%s --help' for more information.",
		appname);
}