#ifndef RESCODE_H
#define RESCODE_H

#define PK_SUCCESS			0	/* Execution success */
#define PK_FILE_EXIST			30	/* File exists */
#define PK_INVALID_PATHNAME		31	/* Invalid pathname */
#define PK_MKDIR_FAILURE		32	/* Make directory failure */
#define PK_MALLOC_FAILURE		33	/* Memory Allocate failure */
#define PK_FILE_INACCESSIBLE		33	/* File cannot be accessed */

#define PK_UNKNOWN_OPTION		60	/* Unknown option */
#define PK_MISSING_OPERAND		61	/* Requires value */
#define PK_INCOMPATIBLE_TYPE		62	/* Incompatible type */
#define PK_TOOMANY_ARGUMENTS		63	/* Too many arguments */
#define PK_UNKNOWN_OPERATION		64	/* Unknown operation */
#define PK_UNCLEAR_OPTARG		65	/* Unclear option-argument */
#define PK_UNSATISFIED_CONDITION	66	/* Requirement not met */

#define ERRMSG_IK			0	/* key, name, field name */
#define ERRMSG_IV			1	/* value */
#define ERRMSG_IE1			2	/* extend value */
#define ERRMSG_IE2			3	/* extend value */

#endif /* RESCODE_H */