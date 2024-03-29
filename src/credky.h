#ifndef CREDKY_H
#define CREDKY_H

struct cipher_config
{
	const char *kdf_algorithm;
	const char *hmac_algorithm;
	unsigned cipher_compat;
	unsigned page_size;
	unsigned kdf_iter;
	uint8_t *key;
	size_t keylen;
	bool is_binary_key;
};

#define CIPHER_DEFAULT_KDF_ALGORITHM	"PBKDF2_HMAC_SHA512"
#define CIPHER_DEFAULT_HMAC_ALGORITHM	"HMAC_SHA512"
#define CIPHER_DEFAULT_COMPATIBILITY	4
#define CIPHER_DEFAULT_PAGE_SIZE	4096
#define CIPHER_DEFAULT_KDF_ITER		256000

#define CIPHER_MIN_COMPATIBILITY	1
#define CIPHER_MAX_COMPATIBILITY	CIPHER_DEFAULT_COMPATIBILITY

#define DIGEST_LENGTH 32

uint8_t *serialize_cipher_config(struct cipher_config *config, size_t *buflen);

#endif /* CREDKY_H */