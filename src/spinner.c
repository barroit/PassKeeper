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

int run_default_spinner(const void *period_mult0)
{
	useconds_t period_mult;
	int i;
	const char *snch[] = { "\\\b", "|\b", "/\b", "-\b" };

	period_mult = *(useconds_t *)period_mult0;
	for (i = 0; ; i = (i + 1) % 4)
	{
		write(STDOUT_FILENO, snch[i], 2);
		usleep(period_mult);
	}

	return 0; /* fake return */
}

static int run_kawaii_spinner(const void *period_mult0)
{
	useconds_t period_mult = *(useconds_t *)period_mult0;

	return period_mult;
}

int run_spinner(struct process_info *ctx, const char *style)
{
	useconds_t period;
	procfn_t spinner_func;

	if (!strcmp(style, "kawaii"))
	{
		period = DEFAULT_SPINNER_PERIOD * 100;
		spinner_func = run_kawaii_spinner;
	}
	else
	{
		period = DEFAULT_SPINNER_PERIOD * 75;
		spinner_func = run_default_spinner;
	}

	return mkprocf(ctx, spinner_func, &period);
}