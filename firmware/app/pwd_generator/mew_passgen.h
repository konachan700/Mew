#ifndef __MEW_PWD_GEN__ 
#define __MEW_PWD_GEN__

#include "mew.h"

#include "../../drivers/flash/flash.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEW_PASSWORD_ALLOW_BASE                 (1 << 1)
#define MEW_PASSWORD_ALLOW_UPPER                (1 << 2)
#define MEW_PASSWORD_ALLOW_SYMS                 (1 << 3)

#define MEW_PASSWORD_MAX_LENGTH                 64

#define MEW_MOD_NO                              0
#define MEW_MOD_UPPERCASE                       1

void mew_xor_keygen(uint8_t* buf, uint32_t size);
uint8_t mew_xor_byte(uint8_t byte, mew_xor_key *key, uint32_t counter);
void mew_xor(uint8_t* in_buf, uint8_t* out_buf, uint32_t size, mew_xor_key *key);

void mew_create_password(mew_keycode* out_buf, mew_xor_key *key, mew_password_config *config);

#endif