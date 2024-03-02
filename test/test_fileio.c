/****************************************************************************
**
** Copyright 2023, 2024 Jiamu Sun
** Contact: barroit@linux.com
**
** This file is part of PassKeeper.
**
** PassKeeper is free software: you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation, either version 3 of the License, or (at your
** option) any later version.
**
** PassKeeper is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with PassKeeper. If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "fileio.h"
#include "rescode.h"
#include <unistd.h>

#define FPATHNAME	"./dummyfile"
#define DPATHNAME	"./dummydir"
#define KEYSTR		"0x0A335A9EC0EE2AED994324AABBAC7EBAB17492D025EAEDDB5E70DAEA499D6587"

void fileio_test_setup(void)
{
	FILE *fs;
	
	fs = fopen(FPATHNAME, "w");
	fputs(KEYSTR, fs);

	fclose(fs);

	rmdir(DPATHNAME);
}

void fileio_test_teardown(void)
{
	remove(FPATHNAME);
	rmdir(DPATHNAME);
}

START_TEST(test_mkdir_p)
{
	mkdir_p(DPATHNAME);
	ck_assert_int_eq(access(DPATHNAME, R_OK | W_OK | X_OK), 0);
}
END_TEST

START_TEST(test_prepare_folder)
{
	ck_assert_int_eq(prepare_folder("test.txt"), PK_INVALID_PATHNAME);

	ck_assert_int_eq(prepare_folder(DPATHNAME"/test.txt"), PK_SUCCESS);

	ck_assert_int_eq(access(DPATHNAME, R_OK | W_OK | X_OK), 0);
}
END_TEST

START_TEST(test_read_content)
{
	size_t sz;
	char *content;

	content = read_content(FPATHNAME, &sz);

	ck_assert_str_eq(content, KEYSTR);
	ck_assert_int_eq(strlen(content), sz);

	free(content);
}
END_TEST

START_TEST(test_prefix)
{
	char *dir;

	ck_assert_ptr_null((dir = prefix("test.a")));

	ck_assert_str_eq((dir = prefix("./dummydir/test.a")), "./dummydir");

	free(dir);
}
END_TEST

Suite *fileio_test_suite(void)
{
	Suite *s;
	TCase *tc_component;

	s = suite_create("fileio");
	tc_component = tcase_create("component");
	tcase_add_checked_fixture(tc_component, fileio_test_setup, fileio_test_teardown);

	tcase_add_test(tc_component, test_mkdir_p);
	tcase_add_test(tc_component, test_prepare_folder);
	tcase_add_test(tc_component, test_read_content);
	tcase_add_test(tc_component, test_prefix);
	suite_add_tcase(s, tc_component);

	return s;
}