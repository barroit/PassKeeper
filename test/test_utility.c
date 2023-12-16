#include "test_utility.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>

START_TEST(test_is_positive_integer)
{
	ck_assert_int_eq(is_positive_integer("123"), 1);
	ck_assert_int_eq(is_positive_integer("1a2s3d"), 0);
	ck_assert_int_eq(is_positive_integer(""), 0);
}
END_TEST

START_TEST(test_is_rw_file)
{
	ck_assert_int_eq(is_rw_file(NULL), 0);
	ck_assert_int_eq(is_rw_file("asd"), 0);

	FILE *fptr;
	fptr = fopen("test.example", "w");
	fclose(fptr);

	ck_assert_int_eq(is_rw_file("test.example"), 1);

	remove("test.example");
}
END_TEST

START_TEST(test_is_empty_string)
{
	ck_assert_int_eq(is_empty_string(NULL), 1);
	ck_assert_int_eq(is_empty_string(""), 1);
	ck_assert_int_eq(is_empty_string("asd"), 0);
}
END_TEST

START_TEST(test_get_space)
{
	char *spaces = NULL;
	ck_assert_int_eq(get_space(&spaces, 5), 0);
	free(spaces);

	spaces = NULL;
	ck_assert_int_eq(get_space(&spaces, -2), 1);
	free(spaces);
}
END_TEST

Suite *utility_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Utility");
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_is_positive_integer);
	tcase_add_test(tc_core, test_is_rw_file);
	tcase_add_test(tc_core, test_is_empty_string);
	tcase_add_test(tc_core, test_get_space);
	suite_add_tcase(s, tc_core);

	return s;
}