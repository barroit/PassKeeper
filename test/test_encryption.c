#include "encrypt.h"
#include <check.h>
#include <string.h>
#include <stdlib.h>

#define KEYLEN 32

START_TEST(test_bin_to_hex)
{
	void *binkey;
	char *hexkey;

	binkey = get_binary_key(KEYLEN);
	hexkey = bin_to_hex(binkey, KEYLEN);

	ck_assert_int_eq(strlen(hexkey), KEYLEN * 2);

	free(binkey);
	free(hexkey);
}
END_TEST

START_TEST(test_hex_to_bin)
{
	void *binkey1, *binkey2;
	char *hexkey;
	size_t binkeylen;

	binkey1 = get_binary_key(KEYLEN);

	hexkey = bin_to_hex(binkey1, KEYLEN);

	binkey2 = hex_to_bin(hexkey, &binkeylen);

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

	tcase_add_test(tc_component, test_bin_to_hex);
	tcase_add_test(tc_component, test_hex_to_bin);
	suite_add_tcase(s, tc_component);

	return s;
}