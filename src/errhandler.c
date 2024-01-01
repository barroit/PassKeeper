#include "errhandler.h"
#include "rescode.h"
#include "utility.h"
#include "strbuffer.h"

#include <stdlib.h>
#include <stdio.h>

extern const char *appname;

#define WRITE_ERR(format, ...) fprintf(stderr, "%s: " format "\n", appname, __VA_ARGS__)

void handle_parse_cmdopts_error(int rc, const char *errmsg[2])
{
	switch (rc)
	{
		case PK_UNKNOWN_OPTION:
			WRITE_ERR("unrecognized option '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_MISSING_OPERAND:
			WRITE_ERR("option '%s' requires an operand", errmsg[ERRMSG_IK]);
			break;
		case PK_INCOMPATIBLE_TYPE:
			WRITE_ERR("incompatible value '%s' converted to option '%s'", STRINGIFY(errmsg[ERRMSG_IV]), errmsg[ERRMSG_IK]);
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
			WRITE_ERR("unrecognized database operation '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_INCOMPATIBLE_TYPE:
			WRITE_ERR("incompatible value '%s' converted to option '%s'", STRINGIFY(errmsg[ERRMSG_IV]), errmsg[ERRMSG_IK]);
			break;
		case PK_UNCLEAR_OPTARG:
			WRITE_ERR("option '%s' redefined with argument value '%s'", errmsg[ERRMSG_IK], STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_TOOMANY_ARGUMENTS:
			WRITE_ERR("too many arguments ('%s'%s%s%s%s) found during parsing",
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
			WRITE_ERR("inaccessed database pathname '%s'", STRINGIFY(errmsg[ERRMSG_IV]));
			break;
		case PK_UNSATISFIED_CONDITION:
			WRITE_ERR("database operation '%s' requires %s%s%s",
				errmsg[ERRMSG_IK],
				errmsg[ERRMSG_IV],
				errmsg[ERRMSG_IE1] == NULL ? "" : " and ",
				errmsg[ERRMSG_IE1] == NULL ? "" : errmsg[ERRMSG_IE1]);
			break;
		default:
			abort();
	}
}

// 

void handle_missing_operation_error(void)
{
	WRITE_ERR("missing database operation\n"
		"Try '%s --help' for more information.",
		appname);
}