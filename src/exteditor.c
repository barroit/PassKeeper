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

#include "pkproc.h"
#include "message.h"
#include "strlist.h"

#define graphical_editor_list		\
	TMP_STRARR(			\
		"gedit",		\
		"gnome-text-editor",	\
		"kate",			\
		"code",			\
		"notepad",		\
		"notepad++",		\
		"subl",			\
		NULL,			\
	)

int edit_file(const char *pathname)
{
	bool show_spinner;

	if (ext_editor == NULL)
	{
		return error("unable to find an editor; make sure VISUAL, "
				"EDITOR or PK_EDITOR is set in env");
	}

	struct process_info
		editor_ctx = {
			.program = ext_editor,
		},
		spinner_ctx = {
			.program = "spinner",
		};

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	if (mkprocl(&editor_ctx, ext_editor, ext_editor, pathname, NULL) != 0)
	{
		return error("unable to launch editor ‘%s’",
				*ext_editor == 0 ? "(empty)" : ext_editor);
	}

	show_spinner = spinner_style != NULL /* --no-spinner */ &&
			/* graphical editor enable this by default */
			 (findstr(ext_editor, graphical_editor_list) ||
			  /* this case, user specified a value */
			   spinner_style != (void *)-1);

	if (show_spinner && run_spinner(&spinner_ctx, spinner_style) != 0)
	{
		note("something terrible happened, but it's harmless, "
			"and the program will continue");

		show_spinner = false;
	}

	finish_process(&editor_ctx, false);

	if (show_spinner)
	{
		kill_process(&spinner_ctx, SIGTERM);
		finish_process(&spinner_ctx, true);
	}

	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);

	return 0;
}
