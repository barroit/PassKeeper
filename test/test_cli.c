#include "test_cli.h"
#include "cli.h"
#include "helper.h"

START_TEST(test_show_version)
{
	SUPPRESS_OUTPUT(show_version());
}
END_TEST

START_TEST(test_show_all_usages)
{
	SUPPRESS_OUTPUT(show_all_usages("pk"));
}
END_TEST

START_TEST(test_show_command_usage)
{
	SUPPRESS_OUTPUT(show_command_usage("pk", "read"));
	SUPPRESS_OUTPUT(show_command_usage("pk", "create"));
	SUPPRESS_OUTPUT(show_command_usage("pk", "update"));
	SUPPRESS_OUTPUT(show_command_usage("pk", "delete"));
	SUPPRESS_OUTPUT(show_command_usage("pk", "C"));
	SUPPRESS_OUTPUT(show_command_usage("pk", "R"));
	SUPPRESS_OUTPUT(show_command_usage("pk", "U"));
	SUPPRESS_OUTPUT(show_command_usage("pk", "D"));
}
END_TEST

START_TEST(test_get_command_usage_format)
{
	ck_assert_ptr_nonnull(get_create_usage_format());
	ck_assert_ptr_nonnull(get_read_usage_format());
	ck_assert_ptr_nonnull(get_update_usage_format());
	ck_assert_ptr_nonnull(get_delete_usage_format());
}
END_TEST

Suite *cli_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Cli");
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_show_version);
	tcase_add_test(tc_core, test_show_all_usages);
	tcase_add_test(tc_core, test_show_command_usage);
	tcase_add_test(tc_core, test_get_command_usage_format);
	suite_add_tcase(s, tc_core);

	return s;
}