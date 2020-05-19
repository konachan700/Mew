#include "mew.h"

#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/crypto.h>
#include <libopencm3/stm32/hash.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "crypto.h"

#define HASH_HR_MEW (&MMIO32(HASH + 0x310))
#define HASH_SWAP(num) (((num & 0xff000000) >> 24) | ((num & 0x00ff0000) >> 8) | ((num & 0x0000ff00) << 8) | (num << 24))

static uint32_t __buf_32 = 0;
static uint8_t* __buf_8 = (uint8_t*) &__buf_32;
static uint32_t __buf_8_counter;

static uint32_t __result_32[8];
static uint8_t* __result_8 = (uint8_t*) __result_32;

void mew_sha256_init(void) {
	__buf_32 = 0;
	__buf_8_counter = 0;

	HASH_CR = HASH_DATA_8BIT | HASH_MODE_HASH | MEW_HASH_ALGO_SHA256;
	HASH_CR |= HASH_CR_INIT;
}

void mew_sha256_add_byte(unsigned char b) {
	__buf_8[__buf_8_counter] = b;
	__buf_8_counter++;
	if (__buf_8_counter >= 4) {
		HASH_DIN = __buf_32;
		while(HASH_SR && HASH_SR_BUSY) __asm__("NOP");
		__buf_32 = 0;
		__buf_8_counter = 0;
	}
}

void mew_sha256_finalize(uint8_t* buffer) {
	int validbits = 0, i;
	if (__buf_8_counter > 0) {
		HASH_DIN = __buf_32;
		while(HASH_SR && HASH_SR_BUSY) __asm__("NOP");
	}
	HASH_STR = (1 << 8);
	while(HASH_SR && HASH_SR_BUSY) __asm__("NOP");
	for (i=0; i<8; i++) {
		__result_32[i] = HASH_SWAP(HASH_HR_MEW[i]);
	}
	memcpy(buffer, __result_8, 32);
}

