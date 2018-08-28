#ifndef MEW_CRYPTO
#define MEW_CRYPTO

#include "../config.h"

#define MEW_HASH_BLOCK_SIZE 			16
#define MEW_HASH_SIZE				8
#define MEW_HASH_ALGO_SHA256			((1 << 7) | (1 << 18))

#define MEW_PASSWORD_ALLOW_BASE         (1 << 1)
#define MEW_PASSWORD_ALLOW_UPPER        (1 << 2)
#define MEW_PASSWORD_ALLOW_SYMS         (1 << 3)

#define MEW_PASSWORD_MAX_LENGTH         64

#define MEW_MOD_NO          0
#define MEW_MOD_UPPERCASE   1

#define MEW_ENCRYPT         1
#define MEW_DECRYPT         2

typedef struct {
    uint64_t pointer;
    uint8_t  table[16][16];
} mew_xor_key;

typedef struct {
    uint32_t allowed_sym;
    uint8_t length;
} mew_password_config;

typedef struct {
  uint16_t keycode;
  uint8_t  mod;
} mew_keycode;

void mew_start_random(void);
uint32_t mew_random32(void);

void mew_hash32(uint32_t* data, uint32_t size, uint32_t* result);
void mew_hash8(uint8_t* data, uint32_t size, uint8_t* result);

void mew_xor_keygen(mew_xor_key *key);
uint8_t mew_xor_byte(uint8_t byte, mew_xor_key *key, uint32_t counter);
void mew_xor(uint8_t* in_buf, uint8_t* out_buf, uint32_t size, mew_xor_key *key);

void mew_create_password(mew_keycode* out_buf, mew_xor_key *key, mew_password_config *config);

void mewcrypt_aes256(uint8_t dir, uint32_t* in_block, uint32_t* out_block, uint32_t count, uint64_t* key, uint64_t* iv);

#endif