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

Suite *fileio_test_suite(void);

Suite *stringbuffer_test_suite(void);

Suite *encrypt_test_suite(void);

Suite *strutil_test_suite(void);

int main()
{
	int number_failed;
	SRunner *sr;

	sr = srunner_create(fileio_test_suite());
	srunner_add_suite(sr, stringbuffer_test_suite());
	srunner_add_suite(sr, encrypt_test_suite());
	srunner_add_suite(sr, strutil_test_suite());

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}