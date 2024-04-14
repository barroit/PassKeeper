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

static inline bool is_dumb_terminal(void)
{
	const char *term = getenv("TERM");
	return term == NULL || !strcmp(term, "dumb");
}

static const char *get_editor(void)
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

static const char *graphical_editors[] = {
	"gedit",     /* gedit */
	"kate",      /* kate */
	"code",      /* vscode */
	"notepad",   /* notepad */
	"notepad++", /* notepad++ */
	"subl",      /* sublime text */
	NULL,
};

static inline bool is_graphical_editor(const char *editor)
{
	return string_in_array(editor, graphical_editors);
}

int edit_file(const char *pathname)
{
	const char *editor, *spinner_style;
	bool show_spinner;

	if ((editor = get_editor()) == NULL)
	{
		return error("Unable to find a editor; Make sure VISUAL, "
				"EDITOR or PK_EDITOR is set in env");
	}

	struct process_info
		editor_ctx = {
			.program = editor,
		},
		spinner_ctx = {
			.program = "spinner",
		};

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	if (mkprocl(&editor_ctx, editor, editor, pathname, NULL) != 0)
	{
		return error("unable to launch editor '%s'",
				*editor == 0 ? "(empty)" : editor);
	}

	/* with --no-spinner, getenv(PK_SPINNER) returns NULL */
	spinner_style = getenv(PK_SPINNER);

	show_spinner = spinner_style && /* must not be NULL */
			/* graphical editor enable this by default */
			 (is_graphical_editor(editor) ||
			  /* this case, user specified a value */
			   strcmp(spinner_style, "0"));

	if (show_spinner && run_spinner(&spinner_ctx, spinner_style) != 0)
	{
		fputs("note: something terrible happened, but it's harmless, "
			"and the program will continue", stderr);

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