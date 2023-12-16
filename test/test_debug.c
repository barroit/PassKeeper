#include "test_debug.h"
#include "parse_command.h"
#define DEBUG
#include "debug.h"

START_TEST(test_print_appopt)
{
	struct app_option appopt = {
		NULL,
		NULL,
		NULL,
		NULL,
		"NULL",
		"NULL",
		NULL,
		NULL,
		NULL,
		-1,
		0,
		0,
		0
	};

	print_appopt(&appopt);
}
END_TEST

Suite *debug_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Debug");
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_print_appopt);
	suite_add_tcase(s, tc_core);

	return s;
}