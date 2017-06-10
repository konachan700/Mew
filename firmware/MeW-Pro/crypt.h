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
#include <libopencm3/stm32/flash.h>

#include "board.h" 
#include "sdcard.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MEW_CRYPT_OK        1
#define MEW_CRYPT_ERROR     2

#define MEW_ENCRYPT         1
#define MEW_DECRYPT         2

#define CRYP_IVR_MEW(i) MMIO64(CRYP_BASE + 0x40 + (i) * 8) // libopencm3 bug

#define MAX_PASSWORDS_COUNT (2048 * 1024)
#define RESERVED_SD_SPACE_FOR_PASSWORDS (MAX_PASSWORDS_COUNT * 512) // 1Gbyte in uSD

#define PASSWORD_FLAG_DIRECTORY     (1 << 31)
#define PASSWORD_FLAG_DISABLED      (1 << 30)

#define MEW_SECURITY_INFO_MAGIC 0x12701100UL
#define FLASH_SI_GLOBAL_OFFSET  0x08060000UL // Sector 7 (last 128KByte of flash)

#define FRAM_PAGES_COUNT 8

struct password_record {
    u32 magic;
    u32 id;
    u32 parent_id;
    u32 display_number;
    u8  title[32];
    u8  text[256];
    u8  login[96];
    u8* icon;
    u32 flags;
}; // MAX 512 bytes

struct fram_information {
    u32 magic;
    
}; // MAX 256 bytes

struct security_information {
    u32 magic;
    u64 key[4];
    u64 iv[2];
    u64 uid;
}; 

extern u32 mewcrypt_aes256(u8 dir, u32* plain_block, u32* cipher_block, u32 count);
extern u32 mewcrypt_aes256_gen_keys(void);
extern u32 mewcrypt_sd_block_read(u32 block, u32* data);
extern u32 mewcrypt_sd_block_write(u32 block, u32* data);
extern u32 mewcrypt_fram_page_read(u8 page, u32* data);
extern u32 mewcrypt_fram_page_write(u8 page, u32* data);

#endif
