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

#include "strutil.h"

START_TEST(test_u8substr)
{
	char *res;

	res = u8substr("test", 4, 0);
	ck_assert_ptr_null(res);

	res = u8substr("test", 0, 2);
	ck_assert_str_eq(res, "te");
	free(res);

	res = u8substr("test", 0, 0);
	ck_assert_str_eq(res, "test");
	free(res);

	res = u8substr("漢字", 0, 2);
	ck_assert_str_eq(res, "漢字");
	free(res);

	res = u8substr("漢字", 1, 1);
	ck_assert_str_eq(res, "字");
	free(res);

	res = u8substr("漢字", 0, 0);
	ck_assert_str_eq(res, "漢字");
	free(res);
}
END_TEST

Suite *strutil_test_suite(void)
{
	Suite *s;
	TCase *tc_component;

	s = suite_create("strutil");
	tc_component = tcase_create("component");

	tcase_add_test(tc_component, test_u8substr);
	suite_add_tcase(s, tc_component);

	return s;
}