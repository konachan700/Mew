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

#define PASSWORD_FLAG_DIRECTORY     (1 << 1)
#define PASSWORD_FLAG_DISABLED      (1 << 2)

#define MEW_SETTING_MAGIC 0x12111100UL

#define MEW_SECURITY_INFO_MAGIC 0x12701100UL
#define FLASH_SI_GLOBAL_OFFSET  0x08060000UL // Sector 7 (last 128KByte of flash)

#define FRAM_PAGES_COUNT 8

#define MEW_PASSWORD_RECORD_MAGIC 0x12222143UL
#define MEW_PASSWORD_RECORD_NO_PARENT 0xFFFFFFFFUL

#define MEW_PASSWORD_RECORD_TITLE_LEN   32
#define MEW_PASSWORD_RECORD_TEXT_LEN    64
#define MEW_PASSWORD_EXTRA_SIZE         64

struct settings_record {
    u32 magic;
    u32 global_mode;
    u32 flags;
};

struct settings_eeprom_sector {
    u32 crc32;
    struct settings_record settings;
};

struct password_record {
    u32 magic;                                // 4b
    u32 id;                                   // 4b
    u32 parent_id;                            // 4b
    u32 display_number;                       // 4b
    u8  title[MEW_PASSWORD_RECORD_TITLE_LEN]; // 32b
    u8  text[MEW_PASSWORD_RECORD_TEXT_LEN];   // 64b
    u8  login[MEW_PASSWORD_RECORD_TEXT_LEN];  // 64b
    const u8* icon;                           // 4b
    u32 flags;                                // 4b
    u32 extra[MEW_PASSWORD_EXTRA_SIZE];       // 256b
}; // MAX (512 - 4) bytes

struct password_sector {
    u32 crc32;
    struct password_record password;
};

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
extern u32 mewcrypt_get_pwd_record(struct password_record* pr, u32 index);
extern u32 mewcrypt_write_pr(struct password_record* pr, u32 index);
extern u32 mewcrypt_write_settings(struct settings_record* sr, u8 index);
extern u32 mewcrypt_read_settings(struct settings_record* sr, u8 index);

#endif
