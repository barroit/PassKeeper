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

int get_bias(long *bias)
{
	TIME_ZONE_INFORMATION time_zone;

	switch (GetTimeZoneInformation(&time_zone))
	{
	case TIME_ZONE_ID_UNKNOWN:
		*bias = time_zone.Bias;
		break;
	case TIME_ZONE_ID_STANDARD:
		*bias = time_zone.Bias + time_zone.StandardBias;
		break;
	case TIME_ZONE_ID_DAYLIGHT:
		*bias = time_zone.Bias + time_zone.DaylightBias;
		break;
	default:
		return error("unable to get time zone information");
	}

	*bias = -(*bias / 60);
	return 0;
}
