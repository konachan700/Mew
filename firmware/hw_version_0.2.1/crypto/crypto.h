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





void mew_start_random(void);
void mew_ss_init(void) ;

uint32_t mew_random32(void);
uint64_t mew_random64(void);

void mew_hash32(uint32_t* data, uint32_t size, uint32_t* result);
void mew_hash8(uint8_t* data, uint32_t size, uint8_t* result);

void mew_xor_keygen(uint8_t* buf, uint32_t size);
uint8_t mew_xor_byte(uint8_t byte, mew_xor_key *key, uint32_t counter);
void mew_xor(uint8_t* in_buf, uint8_t* out_buf, uint32_t size, mew_xor_key *key);

void mew_create_password(mew_keycode* out_buf, mew_xor_key *key, mew_password_config *config);

void mewcrypt_aes256(uint8_t dir, uint32_t* in_block, uint32_t* out_block, uint32_t count, uint64_t* key, uint64_t* iv);

#endif