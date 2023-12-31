#ifndef RESCODE_H
#define RESCODE_H

#define PK_SUCCESS		0	/* Execution success */
#define PK_FILE_EXIST		30	/* File exists */
#define PK_INVALID_PATHNAME	31	/* Invalid pathname */
#define PK_MKDIR_FAILURE	32	/* Make directory failure */
#define PK_MALLOC_FAILURE	33	/* Memory Allocate failure */

#define PK_UNKNOWN_OPTION	60	/* Unknown option */
#define PK_MISSING_OPERAND	61	/* Requires value */
#define PK_INCOMPATIBLE_TYPE	62	/* Incompatible type */

#define ERRMSG_IDX_K		0	/* key, name, field name */
#define ERRMSG_IDX_V		1	/* value */

#endif /* RESCODE_H */