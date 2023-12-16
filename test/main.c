#include "test_utility.h"
#include "test_debug.h"
#include <check.h>
#include <stdlib.h>

int main()
{
	int number_failed;
	SRunner *sr;

	sr = srunner_create(utility_suite());
	srunner_add_suite(sr, debug_suite());

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}