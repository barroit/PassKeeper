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
#include "strbuf.h"

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

static const char *graphical_editors[] = { "gedit", "kate", "code", NULL };
static const char *terminal_editors[] = { "vim", "emacs", "nano", NULL };

static bool is_graphical_editor(const char *editor)
{
	if (!strcmp(editor, getenv("VISUAL")))
	{
		return true;
	}
	else if (string_in_array(editor, terminal_editors))
	{
		return false;
	}
	else if (string_in_array(editor, graphical_editors))
	{
		return false;
	}
	else
	{
		/* assuming the user knows what they are doing */
		return true;
	}

}

static int launch_editor(const void *args0)
{
	const char *editor, *tmp_file, **args;

	args = (const char **)args0;
	editor = args[0];
	tmp_file = args[1];

	execlp(editor, editor, tmp_file, NULL);

	return 0;
}

int edit_file(const char *tmp_file)
{
	const char *editor, *spinner_style;
	bool show_spinner;

	if ((editor = get_editor()) == NULL)
	{
		return error("terminal is dumb, but EDITOR unset");
	}

	struct process_info
		editor_ctx = {
			.program = editor,
		},
		spinner_ctx = {
			.program = "spinner",
		};

	if (start_process(&editor_ctx, launch_editor,
		(const char *[]){ editor , tmp_file }) != 0)
	{
		return error("unable to launch editor '%s'", editor);
	}

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	spinner_style = getenv(PK_SPINNER);
	show_spinner = spinner_style && is_graphical_editor(editor);

	if (show_spinner)
	{
		run_spinner(spinner_style, stdout, DEFAULT_SPINNER_PERIOD);
	}

	finish_process(&editor_ctx);

	if (show_spinner)
	{
		kill(spinner_ctx.pid, SIGTERM);
		finish_process(&spinner_ctx);
	}

	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	return 0;
}