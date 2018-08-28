#ifndef MEW_ROOT_CONFIG
#define MEW_ROOT_CONFIG

#include <libopencm3/stm32/f4/nvic.h>
#include <libopencm3/stm32/f4/memorymap.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/assert.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/f4/rng.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/sdio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/msc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/crypto.h>
#include <libopencm3/stm32/hash.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> 

//#define ZRAM __attribute__((section (".zram")))

extern void mew_start_all_clock(void);
extern void mew_systick_setup(void);
extern uint32_t mew_get_millis(void);

void mew_die(void);
void mew_die_with_message(char* str);
void mew_check_dma_memory(void* pointer, char* where);

void mew_delay_ms(uint32_t ms);
void mew_wait_for_state(volatile uint8_t* var, uint8_t expected_state);

#define INTEGRATION_TESTS

#define __MEW_CONFIG__TS_USE_TSC2007
//#define __MEW_CONFIG__TS_USE_ADC

#define __MEW_CONFIG__DISPLAY_USE_DMA

#endif
