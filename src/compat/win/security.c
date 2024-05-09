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

#include "security.h"

int termios_disable_echo(struct termios *term0)
{
	HANDLE stdin_handle;
	DWORD console_mode;

	if ((stdin_handle = GetStdHandle(STD_INPUT_HANDLE)) ==
		INVALID_HANDLE_VALUE)
	{
		warning_winerr("Unable to get stdin handle");
		return -1;
	}

	if (GetConsoleMode(stdin_handle, &console_mode) == 0)
	{
		warning_winerr("Unable to get console mode");
		return -1;
	}

	term0->stdin_handle = stdin_handle;
	term0->console_mode = console_mode;

	console_mode &= ~ENABLE_ECHO_INPUT;
	if (SetConsoleMode(stdin_handle, console_mode) == 0)
	{
		warning_winerr("Unable to apply changes to console");
		return -1;
	}

	return 0;
}

int termios_restore_config(struct termios *term0)
{
	if (SetConsoleMode(term0->stdin_handle, term0->console_mode) == 0)
	{
		warning_winerr("Unable to restore changes to console");
		return -1;
	}

	return 0;
}
