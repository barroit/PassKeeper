#include "errhandler.h"
#include "rescode.h"
#include "utility.h"

#include <stdlib.h>
#include <stdio.h>

#define WRITE_ERR(format, ...) fprintf(stderr, format "\n", __VA_ARGS__)

void handle_parse_cmdopts_error(int rc, const char *error_messages[2])
{
	switch (rc)
	{
		case PK_INCOMPATIBLE_TYPE:
			WRITE_ERR("cannot convert '%s' to %s: type incompatible", STRINGIFY(error_messages[ERRMSG_IDX_V]), error_messages[ERRMSG_IDX_K]);
			break;
		case PK_MISSING_OPERAND:
			WRITE_ERR("option '%s' requires an argument", error_messages[ERRMSG_IDX_K]);
			break;
		case PK_UNKNOWN_OPTION:
			WRITE_ERR("unrecognized option '%s'", error_messages[ERRMSG_IDX_K]);
			break;
		default:
			abort();
	}
}