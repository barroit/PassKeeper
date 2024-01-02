#define FPATHNAME	"./dummyfile"
#define DPATHNAME	"./dummydir"
#define KEYSTR		"0x0A335A9EC0EE2AED994324AABBAC7EBAB17492D025EAEDDB5E70DAEA499D6587"

#include <stdio.h>

#include <unistd.h>

void io_test_setup(void)
{
	FILE *fs;
	
	fs = fopen(FPATHNAME, "w");
	fputs(KEYSTR, fs);

	fclose(fs);

	rmdir(DPATHNAME);
}

void io_test_teardown(void)
{
	remove(FPATHNAME);
	rmdir(DPATHNAME);
}

#include "io.h"

#include <check.h>

START_TEST(test_mkdir)
{
	dirmake(DPATHNAME);
	ck_assert_int_eq(access(DPATHNAME, R_OK | W_OK | X_OK), 0);
}
END_TEST

#include "rescode.h"

START_TEST(test_prepare_file_folder)
{
	ck_assert_int_eq(prepare_file_folder("test.txt"), PK_INVALID_PATHNAME);

	ck_assert_int_eq(prepare_file_folder(DPATHNAME"/test.txt"), PK_SUCCESS);

	ck_assert_int_eq(access(DPATHNAME, R_OK | W_OK | X_OK), 0);
}
END_TEST

#include <string.h>

#include <stdlib.h>

START_TEST(test_read_file_content)
{
	size_t sz;
	char *content;

	content = read_file_content(FPATHNAME, &sz);

	ck_assert_str_eq(content, KEYSTR);
	ck_assert_int_eq(strlen(content), sz);

	free(content);
}
END_TEST

Suite *io_test_suite(void)
{
	Suite *s;
	TCase *tc_component;

	s = suite_create("io");
	tc_component = tcase_create("component");
	tcase_add_checked_fixture(tc_component, io_test_setup, io_test_teardown);

	tcase_add_test(tc_component, test_mkdir);
	tcase_add_test(tc_component, test_prepare_file_folder);
	tcase_add_test(tc_component, test_read_file_content);
	suite_add_tcase(s, tc_component);

	return s;
}