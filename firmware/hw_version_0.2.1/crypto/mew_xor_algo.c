#include "crypto.h"

/***************************************************************************************************
 *  WARNING!
 *  IT'S NOT CRYPTOGRAPHY. Do not use this module for encrypt any sensitive data.
 *  This module is used for password generator and eeprom configuration scrambling.
****************************************************************************************************/

static void __mew_xor_randomize_array(uint8_t* buf, uint32_t size);

void mew_xor_keygen(mew_xor_key *key) {
    __mew_xor_randomize_array((uint8_t*) key->table, 256);
}

void mew_xor(uint8_t* in_buf, uint8_t* out_buf, uint32_t size, mew_xor_key *key) {
    uint32_t i;
    for (i=0; i<size; i++) {
        out_buf[i] = mew_xor_byte(in_buf[i], key, i);
    }
}

static void __mew_xor_randomize_array(uint8_t* buf, uint32_t size) {
    uint8_t random;
    uint32_t i, counter = 0;
    memset(buf, 0, size);
    while(1) {
        random = (uint8_t) (mew_random32() & 0xFF);
        for (i=0; i<size; i++) {
            if (buf[i] == random) {
                continue;
            }
        }
        buf[counter] = random;
        counter++;
        if (counter >= size) {
            return;
        }
    }
}

uint8_t mew_xor_byte(uint8_t byte, mew_xor_key *key, uint32_t counter) {
    uint8_t i, k, addr_high, addr_low, result = byte;
    for (i=0, k=0; i<32; i+=4, k++) {
        addr_low  = 0x0F & ((key->pointer + counter) >> i);
        addr_high = 0x0F & ((key->pointer + counter) >> (i + 32));
        result = result ^ key->table[k][addr_low] ^ key->table[k + 8][addr_high];
    }
    return result;
}

