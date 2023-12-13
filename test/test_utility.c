#include "test_utility.h"
#include "utility.h"

START_TEST(test_is_positive_integer)
{
	ck_assert_int_eq(is_positive_integer("123"), 1);
	ck_assert_int_eq(is_positive_integer("1a2s3d"), 0);
	ck_assert_int_eq(is_positive_integer(""), 0);
}
END_TEST

Suite *utility_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Utility");
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_is_positive_integer);
	suite_add_tcase(s, tc_core);

	return s;
}