#include "misc.h"
#include <check.h>
#include <stdlib.h>

START_TEST(test_prefix)
{
	char *dir;

	ck_assert_ptr_null((dir = prefix("test.a")));

	ck_assert_str_eq((dir = prefix("./dummydir/test.a")), "./dummydir");

	free(dir);
}
END_TEST

Suite *utility_test_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("utility");
	tc_core = tcase_create("core");

	tcase_add_test(tc_core, test_prefix);
	suite_add_tcase(s, tc_core);

	return s;
}