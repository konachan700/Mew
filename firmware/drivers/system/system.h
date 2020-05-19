#ifndef __MEW_SYSTEM__
#define __MEW_SYSTEM__

#include "mew.h"

#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/f4/rng.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEW_SYSTICK_RELOAD_VALUE 168000

void mew_check_dma_memory(void* pointer, char* where);
void mew_wait_for_state(volatile uint8_t* var, uint8_t expected_state);
void mew_delay_ms(uint32_t ms);

unsigned int mew_get_millis(void);

unsigned int mew_systick_init(void);
unsigned int mew_clock_init(void);
unsigned int mew_rng_init(void);

uint32_t mew_random32(void);
uint64_t mew_random64(void);

#endif