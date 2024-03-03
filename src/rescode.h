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

#ifndef RESCODE_H
#define RESCODE_H

#define PK_SUCCESS			0


#define PK_INVALID_PATHNAME		30
#define PK_FILE_EXISTS			31
#define PK_MISSING_FILE			32
#define PK_PERMISSION_DENIED		33
#define PK_MKDIR_FAILURE		34


#define PK_INVALID_KEY			50
#define PK_KEYGEN_FAILURE		51	/* Key generation failure */
#define PK_INCORRECT_KEY		52


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