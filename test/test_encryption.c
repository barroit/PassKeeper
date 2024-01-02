#define KEYLEN 32

#include "encrypt.h"

#include <check.h>

#include <string.h>

#include <stdlib.h>

START_TEST(test_btoh)
{
	void *binkey;
	char *hexkey;

	binkey = genbytes(KEYLEN);
	hexkey = btoh(binkey, KEYLEN);

	ck_assert_int_eq(strlen(hexkey), KEYLEN * 2);

	free(binkey);
	free(hexkey);
}
END_TEST

START_TEST(test_htob)
{
	void *binkey1, *binkey2;
	char *hexkey;
	size_t binkeylen;

	binkey1 = genbytes(KEYLEN);

	hexkey = btoh(binkey1, KEYLEN);

	binkey2 = htob(hexkey, &binkeylen);

	ck_assert_int_eq(KEYLEN, binkeylen);
	ck_assert_mem_eq(binkey1, binkey2, binkeylen);

	free(binkey1);
	free(binkey2);
	free(hexkey);
}
END_TEST

Suite *encrypt_test_suite(void)
{
	Suite *s;
	TCase *tc_component;

	s = suite_create("encrypt");
	tc_component = tcase_create("component");

	tcase_add_test(tc_component, test_btoh);
	tcase_add_test(tc_component, test_htob);
	suite_add_tcase(s, tc_component);

	return s;
}