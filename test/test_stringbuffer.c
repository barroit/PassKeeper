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

#include "strbuf.h"

struct strbuf *sb;

void stringbuffer_test_setup(void)
{
	strbuf = sballoc(80);
}

void stringbuffer_test_teardown(void)
{
	sbfree(strbuf);
}

START_TEST(test_sbputc)
{
	const char *str, *iter;

	str = "hello world >_<";
	iter = str;

	while (*iter != '\0')
	{
		sbputc(strbuf, *iter++);
	}

	ck_assert_str_eq(strbuf->data, str);
	ck_assert_int_eq(strbuf->size, strlen(str));
}
END_TEST

START_TEST(test_sbprint)
{
	const char **iter, *str, *strarr[5] = {
		"Cras non metus vitae enim sollicitudin sollicitudin nec auctor libero. "
		"Ut tempus lorem porta tortor cursus pharetra. ",
		"Fusce tellus erat, volutpat sit amet tristique vel, venenatis eu leo. ",
		"Integer sollicitudin odio arcu, nec faucibus risus luctus at.",
		NULL,
	};

	str = "Cras non metus vitae enim sollicitudin sollicitudin nec auctor libero. Ut tempus lorem porta tortor cursus pharetra. Fusce tellus erat, volutpat sit amet tristique vel, venenatis eu leo. Integer sollicitudin odio arcu, nec faucibus risus luctus at.";

	iter = strarr;
	while (*iter != NULL)
	{
		sbprint(strbuf, *iter++);
	}

	ck_assert_str_eq(strbuf->data, str);
	ck_assert_int_eq(strbuf->size, strlen(str));
}
END_TEST

START_TEST(test_sbprintf)
{
	const char *str;

	sbprintf(strbuf, "Cras n%sae enim sol%sudin nec auctor libero. ", "on metus vit", "licitudin sollicit");
	sbprintf(strbuf, "%ct tempus lor%sta tortor cursus pharetra.", 'U', "em por");

	str = "Cras non metus vitae enim sollicitudin sollicitudin nec auctor libero. Ut tempus lorem porta tortor cursus pharetra.";
	ck_assert_str_eq(strbuf->data, str);
	ck_assert_int_eq(strbuf->size, strlen(str));
}
END_TEST

START_TEST(test_sbnprintf)
{
	const char *str;

	/* write entire string into buffer */
	str = "Cras non metus vitae enim sollicitudin sollicitudin nec auctor libero. ";
	sbnprintf(strbuf, strlen(str), "Cras non me%sudin sollicitudin nec auctor libero. ", "tus vitae enim sollicit");

	ck_assert_str_eq(strbuf->data, str);
	ck_assert_int_eq(strbuf->size, strlen(str));

	/* write limited length of string into buffer */
	str = "Cras non metus vitae enim sollicitudin sollicitudin nec auctor libero. Ut tempus lorem po";
	sbnprintf(strbuf, 18, "Ut te%sem p%sor cursus pharetra.", "mpus lor", "orta tort");

	ck_assert_str_eq(strbuf->data, str);
	ck_assert_int_eq(strbuf->size, strlen(str));

	/* write entire string into buffer with bufsz limit much larger than the string length */
	str = "Cras non metus vitae enim sollicitudin sollicitudin nec auctor libero. Ut tempus lorem porta tort";
	sbnprintf(strbuf, 100, "rt%s%s", "a to", "rt");

	ck_assert_str_eq(strbuf->data, str);
	ck_assert_int_eq(strbuf->size, strlen(str));
}
END_TEST

Suite *stringbuffer_test_suite(void)
{
	Suite *s;
	TCase *tc_structure;

	s = suite_create("stringbuffer");
	tc_structure = tcase_create("structure");
	tcase_add_checked_fixture(tc_structure, stringbuffer_test_setup, stringbuffer_test_teardown);

	tcase_add_test(tc_structure, test_sbputc);
	tcase_add_test(tc_structure, test_sbprint);
	tcase_add_test(tc_structure, test_sbprintf);
	tcase_add_test(tc_structure, test_sbnprintf);
	suite_add_tcase(s, tc_structure);

	return s;
}