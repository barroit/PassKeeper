#ifndef CREDKY_H
#define CREDKY_H

struct cipher_config
{
	const char *kdf_algorithm;
	const char *hmac_algorithm;
	unsigned   cipher_compat;
	unsigned   page_size;
	unsigned   kdf_iter;
};

/* CPRDEF stands for cipher default */
#define CPRDEF_KDF_ALGORITHM  "PBKDF2_HMAC_SHA512"
#define CPRDEF_HMAC_ALGORITHM "HMAC_SHA512"
#define CPRDEF_COMPATIBILITY  4
#define CPRDEF_PAGE_SIZE      4096
#define CPRDEF_KDF_ITER       256000

#define CPRMIN_COMPATIBILITY 1
#define CPRMAX_COMPATIBILITY CPRDEF_COMPATIBILITY

#define CPRMIN_PAGE_SIZE 512
#define CPRMAX_PAGE_SIZE 65536

#define DIGEST_LENGTH 32

/**
 * this function guarantees the returned buffer
 * has enough space to place the message digest
 */
uint8_t *serialize_cipher_config(struct cipher_config *config, const uint8_t *key, size_t keylen, bool is_binary_key, size_t *outlen);

#endif /* CREDKY_H */
