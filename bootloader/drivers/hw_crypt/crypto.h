#ifndef MEW_CRYPTO
#define MEW_CRYPTO

#include "mew.h"

#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/crypto.h>
#include <libopencm3/stm32/hash.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEW_HASH_BLOCK_SIZE 			16
#define MEW_HASH_SIZE					8
#define MEW_HASH_ALGO_SHA256			((1 << 7) | (1 << 18))

#define MEW_ENCRYPT         1
#define MEW_DECRYPT         2

#define MEW_PINCODE_LEN 	64

void mew_hash32(uint32_t* data, uint32_t size, uint32_t* result);
void mew_hash8(uint8_t* data, uint32_t size, uint8_t* result);

void mewcrypt_aes256(uint8_t dir, uint32_t* in_block, uint32_t* out_block, uint32_t count, const uint64_t* key, const uint64_t* iv);

#endif
