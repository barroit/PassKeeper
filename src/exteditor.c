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

#include "exteditor.h"

const char *get_editor(void)
{
	const char *editor = getenv(PK_EDITOR);
	bool dumb = is_dumb_terminal();

	if (editor == NULL && !dumb)
	{
		editor = getenv("VISUAL");
	}
	if (editor == NULL)
	{
		editor = getenv("EDITOR");
	}

	if (editor == NULL)
	{
		editor = dumb ? NULL : DEFAULT_EDITOR;
	}

	return editor;
}
// int run_process()
// {
// 	pid_t pid;

// 	if (pid == 0)
// 	{
// 		execlp(editor, editor, path, NULL);

// 		// errno;
// 	}
// }

// int run_spinner(void)
// {
// 	return 0;
// }

// int launch_editor(const char *path)
// {
// 	const char *editor;
// 	pid_t pid;

// 	if ((editor = get_editor()) == NULL)
// 	{
// 		return error("terminal is dumb, but EDITOR unset");
// 	}

// 	pid = fork();

// 	if (pid == 0)
// 	{
// 		execlp(editor, editor, path, NULL);

// 		// errno;
// 	}
// 	else
// 	{
// 		wait(NULL);
// 	}

// 	return 0;
// }

// int edit_file(const char *file_path)
// {
// 	bool show_spinner;

// 	launch_editor(file_path);

// 	show_spinner = !!getenv(PK_SPINNER);
// 	if (show_spinner)
// 	{
// 		run_spinner();
// 	}

// 	//
// 	return 0;
// }