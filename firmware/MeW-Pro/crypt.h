#ifndef __BOARD_MEW_CRYPTO__
#define __BOARD_MEW_CRYPTO__

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/assert.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/f2/rng.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/sdio.h>
#include <libopencm3/stm32/crypto.h>

#include "board.h" 
#include "sdcard.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MEW_ENCRYPT 1
#define MEW_DECRYPT 2

extern u32 mewcrypt_aes256(u8 dir, u32* plain_block, u32* cipher_block, u32 count);
extern u32 mewcrypt_aes256_gen_testkeys(void);

#endif
