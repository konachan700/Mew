#ifndef __MEW_FLASH__ 
#define __MEW_FLASH__

#include "mew.h"

#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/crc.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEW_MAGIC           0x01EF7743
#define MEW_MAGIC_TSIZE     16

typedef struct {
    uint64_t pointer;
    uint8_t  table[MEW_MAGIC_TSIZE][MEW_MAGIC_TSIZE];
} mew_xor_key;

typedef struct {
    uint32_t allowed_sym;
    uint8_t length;
} mew_password_config;

typedef struct {
  uint16_t keycode;
  uint8_t  mod;
} mew_keycode;

typedef struct {
    uint32_t crc32;
    uint8_t  table[MEW_MAGIC_TSIZE][MEW_MAGIC_TSIZE];
} mew_xor_key_rom;

typedef struct {
    uint32_t magic;
    mew_xor_key_rom master_key;
    mew_xor_key_rom passwords_key;
    uint64_t system_encrypt_key[4];
    uint64_t system_encrypt_iv[2];
    
} mew_unique_data;

unsigned int mew_is_first_start(void);
unsigned int mew_otp_write_temporary_data(void);

unsigned int mew_otp_storage_init(void);

#endif
