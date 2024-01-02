#include <check.h>

Suite *io_test_suite(void);

Suite *utility_test_suite(void);

Suite *stringbuffer_test_suite(void);

Suite *encrypt_test_suite(void);

#include <stdlib.h>

int main()
{
	int number_failed;
	SRunner *sr;

	sr = srunner_create(io_test_suite());
	srunner_add_suite(sr, utility_test_suite());
	srunner_add_suite(sr, stringbuffer_test_suite());
	srunner_add_suite(sr, encrypt_test_suite());

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}