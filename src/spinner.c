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

struct spinner_config
{
	FILE *stream;
	useconds_t period_mult; /* 10 == 1x */
};

static int run_default_spinner(const void *rcfg)
{
	return 0;
}

static int run_kawaii_spinner(const void *rcfg)
{
	return 0;
}

void run_spinner(const char *style, FILE *stream, useconds_t period)
{
	struct process_info ctx = { 0 };
	struct spinner_config cfg = {
		.stream = stream,
		.period_mult = period,
	};

	if (!strcmp(style, "kawaii"))
	{
		start_process(&ctx, run_kawaii_spinner, &cfg);
	}
	else
	{
		//
	}
}