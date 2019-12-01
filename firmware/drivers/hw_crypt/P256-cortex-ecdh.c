#include "P256-cortex-ecdh.h"
#include "../system/system.h"
#include "debug.h"

#define MEW_PUBKEY_SIZE 64

static uint8_t public_key[MEW_PUBKEY_SIZE];
static uint8_t private_key[32];
static uint8_t shared_key[32];

static void createRandomPKey(void) {
	uint32_t i;
	for (i=0; i<32; i++) {
		private_key[i] = (uint8_t) mew_random32();
	}
}

unsigned int mew_p256_ecdh_set_session_sharedkey(char* key) {
	memcpy(shared_key, key, 32);
	return 32;
}

unsigned int mew_p256_ecdh_get_session_sharedkey(char* key) {
	memcpy(key, shared_key, 32);
	return 32;
}

unsigned int mew_p256_ecdh_get_session_privkey(char* key) {
	memcpy(key, private_key, 32);
	return 32;
}

unsigned int mew_p256_ecdh_get_session_pubkey(char* key) {
	memcpy(key, public_key, MEW_PUBKEY_SIZE);
	return 64;
}

unsigned int mew_p256_ecdh_handler(void) {
	do {
		createRandomPKey();
	} while (!P256_ecdh_keygen(public_key, private_key));

//	mew_debug_print_hex((const char*)public_key, 64);
//	mew_debug_print_hex((const char*)private_key, 32);
	return 0;
}

unsigned int mew_p256_ecdh_test(void) {
#ifdef __MEW_FACTORY_TESTS__
	uint8_t
		private_key_1[32],
		private_key_2[32],
		shared_1[32],
		shared_2[32],
		public_key_1[64],
		public_key_2[64];

	uint32_t i, k = 0;
	do {
		for (i=0; i<32; i++) private_key_1[i] = (uint8_t) mew_random32();
	} while (!P256_ecdh_keygen(public_key_1, private_key_1));

	do {
		for (i=0; i<32; i++) private_key_2[i] = (uint8_t) mew_random32();
	} while (!P256_ecdh_keygen(public_key_2, private_key_2));

	if (!P256_ecdh_shared_secret(shared_1, public_key_2, private_key_1)) {
		mew_debug_print("mew_p256_ecdh_test: s1 pub2 priv1 fail");
		return 1;
	}

	if (!P256_ecdh_shared_secret(shared_2, public_key_1, private_key_2)) {
		mew_debug_print("mew_p256_ecdh_test: s2 pub1 priv2 fail");
		return 2;
	}

	for (i=0; i<32; i++) {
		if (shared_1[i] != shared_2[i]) k = 1;
	}

	if (k != 0) {
		mew_debug_print("mew_p256_ecdh_test: shared secrets are different");
		return 2;
	}

	mew_debug_print("mew_p256_ecdh_test: ok");
#endif
	return 0;
}

