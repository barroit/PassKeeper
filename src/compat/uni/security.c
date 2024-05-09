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
	struct termios term;

	if (tcgetattr(STDIN_FILENO, &term) == -1)
	{
		warning_errno("Disable echo failed since we "
				"can't get terminal settings");
		return -1;
	}
	*term0 = term;

	term.c_lflag &= ~ECHO;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1)
	{
		warning_errno("Unable to apply changes to terminal");
		return -1;
	}

	return 0;
}

int termios_restore_config(struct termios *term0)
{
	if (tcsetattr(STDIN_FILENO, TCSANOW, term0) == -1)
	{
		warning_errno("Unable to restore changes to terminal");
		return -1;
	}

	return 0;
}
