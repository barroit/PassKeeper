#include "test_cmdparser.h"
#include "cmdparser.h"
#include "helper.h"

START_TEST(test_validate_field)
{
	struct app_option appopt = { NULL, NULL, NULL, NULL, "NULL", "NULL", NULL, NULL, NULL, -1, 0, 0, 0 };
	char *missing_field = NULL;

	MOCK_FILE("a.file", {
		appopt.db_filename = "b.file";
		ck_assert_int_eq(validate_field(&missing_field, &appopt), FILE_INACCESS);
	});

	MOCK_FILE("b.file", {
		appopt.command = "create";
		ck_assert_int_eq(validate_field(&missing_field, &appopt), MISSING_FIELD);
		ck_assert_str_eq(missing_field, "site_name, username or password");
		appopt.site_name = "a site name";
		ck_assert_int_eq(validate_field(&missing_field, &appopt), MISSING_FIELD);
		ck_assert_str_eq(missing_field, "username or password");
		appopt.site_name = NULL;
		appopt.username = "barroit";
		ck_assert_int_eq(validate_field(&missing_field, &appopt), MISSING_FIELD);
		ck_assert_str_eq(missing_field, "site_name");

		appopt.site_name = NULL;
		appopt.command = "R";
		ck_assert_int_eq(validate_field(&missing_field, &appopt), MISSING_FIELD);
		ck_assert_str_eq(missing_field, "site_name");
		appopt.site_name = "a site name";
		ck_assert_int_eq(validate_field(&missing_field, &appopt), 0);
	
		appopt.command = "U";
		ck_assert_int_eq(validate_field(&missing_field, &appopt), MISSING_FIELD);
		ck_assert_str_eq(missing_field, "id");
		appopt.command = "delete";
		appopt.record_id = 0;
		ck_assert_int_eq(validate_field(&missing_field, &appopt), 0);

		appopt.command = "create";
		ck_assert_int_eq(validate_field(&missing_field, &appopt), 0);
	});
}
END_TEST

Suite *cmdparser_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("ParseCommand");
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_validate_field);
	suite_add_tcase(s, tc_core);

	return s;
}