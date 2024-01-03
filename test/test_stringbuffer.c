#include "strbuffer.h"
#include <check.h>
#include <string.h>

string_buffer *buf;

void stringbuffer_test_setup(void)
{
	buf = sbmake(80);
}

void stringbuffer_test_teardown(void)
{
	sbfree(buf);
}

START_TEST(test_sbputc)
{
	const char *str, *iter;

	str = "hello world >_<";
	iter = str;

	while (*iter != '\0')
	{
		sbputc(buf, *iter++);
	}

	ck_assert_str_eq(buf->data, str);
	ck_assert_int_eq(buf->size, strlen(str));
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
		sbprint(buf, *iter++);
	}

	ck_assert_str_eq(buf->data, str);
	ck_assert_int_eq(buf->size, strlen(str));
}
END_TEST

START_TEST(test_sbprintf)
{
	const char *str;

	sbprintf(buf, "Cras n%sae enim sol%sudin nec auctor libero. ", "on metus vit", "licitudin sollicit");
	sbprintf(buf, "%ct tempus lor%sta tortor cursus pharetra.", 'U', "em por");

	str = "Cras non metus vitae enim sollicitudin sollicitudin nec auctor libero. Ut tempus lorem porta tortor cursus pharetra.";
	ck_assert_str_eq(buf->data, str);
	ck_assert_int_eq(buf->size, strlen(str));
}
END_TEST

START_TEST(test_sbnprintf)
{
	const char *str;

	str = "Cras non metus vitae enim sollicitudin sollicitudin nec auctor libero. ";
	sbnprintf(buf, strlen(str), "Cras non me%sudin sollicitudin nec auctor libero. ", "tus vitae enim sollicit");

	ck_assert_str_eq(buf->data, str);
	ck_assert_int_eq(buf->size, strlen(str));

	str = "Cras non metus vitae enim sollicitudin sollicitudin nec auctor libero. Ut tempus lorem po";
	sbnprintf(buf, 18, "Ut te%sem p%sor cursus pharetra.", "mpus lor", "orta tort");

	ck_assert_str_eq(buf->data, str);
	ck_assert_int_eq(buf->size, strlen(str));
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