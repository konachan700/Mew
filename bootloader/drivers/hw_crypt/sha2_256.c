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

void mew_hash32(uint32_t* data, uint32_t size, uint32_t* result) {
	uint32_t i;

	HASH_CR = HASH_DATA_32BIT | HASH_MODE_HASH | MEW_HASH_ALGO_SHA256;
	HASH_CR |= HASH_CR_INIT;

	for (i=0; i<size; i++) {
            HASH_DIN = data[i];
            while(HASH_SR && HASH_SR_BUSY) __asm__("NOP");
	}
	HASH_STR = (1 << 8);
	while(HASH_SR && HASH_SR_BUSY) __asm__("NOP");

	for (i=0; i<MEW_HASH_SIZE; i++) {
            result[i] = HASH_HR_MEW[i];
	}
}

void mew_hash8(uint8_t* data, uint32_t size, uint8_t* result) {
	uint32_t i;
	uint32_t data_tmp;
	uint32_t tail = size % sizeof(uint32_t);
	uint32_t aligned_size = size - tail;

	HASH_CR = HASH_DATA_8BIT | HASH_MODE_HASH | MEW_HASH_ALGO_SHA256;
	HASH_CR |= HASH_CR_INIT;

	for (i=0; i<aligned_size; i+=4) {
            memcpy(&data_tmp, data + i, sizeof(uint32_t));
            HASH_DIN = data_tmp;
            while(HASH_SR && HASH_SR_BUSY) __asm__("NOP");
	}

	if (tail > 0) {
            data_tmp = 0;
            memcpy(&data_tmp, data + aligned_size, tail);
            HASH_DIN = data_tmp;
            while(HASH_SR && HASH_SR_BUSY) __asm__("NOP");
	}

	HASH_STR = (1 << 8);
	while(HASH_SR && HASH_SR_BUSY) __asm__("NOP");

	for (i=0; i<MEW_HASH_SIZE; i++) {
            result[i] = HASH_HR_MEW[i];
	}
}
