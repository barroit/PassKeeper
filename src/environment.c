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

#include "environment.h"
#include "projcfg.h"

const char *credential_path;
const char *credential_key_path;

bool is_encrypt;

struct project_info projinfo;

void initialize_environment(void)
{
	credential_path = getenv(CREDENTIAL_ENVNAME);
	credential_key_path = getenv(CREDENTIAL_KEY_ENVNAME);

	is_encrypt = credential_key_path != NULL;

	projinfo.name = PROJECT_NAME;
	projinfo.author = AUTHOR_NAME;
	projinfo.contact = CONTACT_ADDRESS;
}