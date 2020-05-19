#ifndef MEW_CRYPTO
#define MEW_CRYPTO

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEW_HASH_BLOCK_SIZE 			16
#define MEW_HASH_SIZE					8
#define MEW_HASH_ALGO_SHA256			((1 << 7) | (1 << 18))

#define MEW_ENCRYPT         1
#define MEW_DECRYPT         2

#define MEW_PINCODE_LEN 	64

void mew_sha256_init(void);
void mew_sha256_add_byte(unsigned char b);
void mew_sha256_finalize(uint8_t* buffer);

void mewcrypt_aes256(uint8_t dir, uint32_t* in_block, uint32_t* out_block, uint32_t count, const uint64_t* key, const uint64_t* iv);

#endif
