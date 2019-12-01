#include "mew_passgen.h"
#include "../keyboard/input-event-codes.h"

static void __mew_fill_keycode(uint8_t b, mew_keycode* out_buf, mew_password_config *config);

#define MEW_LETTERS_COUNT 26
static const uint16_t keycodes_letter[] = {
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, 
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, 
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M 
};

#define MEW_DIGITS_COUNT 10
static const uint16_t keycodes_digits[] = {
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0
};

#define MEW_SYMBOLS_COUNT 11
static const uint16_t keycodes_symbols[] = {
    KEY_MINUS, KEY_EQUAL, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE,
    KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_BACKSLASH
};

void mew_create_password(mew_keycode* out_buf, mew_xor_key *key, mew_password_config *config) {
    uint8_t i, b;
    
    memset((void*)out_buf, 0, (config->length * sizeof(mew_keycode)));
    if (config->length > MEW_PASSWORD_MAX_LENGTH) {
        return;
    }
    
    for (i=0; i<config->length; i++) {
        b = mew_xor_byte(0, key, i);
        __mew_fill_keycode(b, &out_buf[i], config);
    }
}

static void __mew_fill_keycode(uint8_t b, mew_keycode* out_buf, mew_password_config *config) {
    uint16_t keycode; 
    if (config->allowed_sym && MEW_PASSWORD_ALLOW_UPPER) {
        if (config->allowed_sym && MEW_PASSWORD_ALLOW_SYMS) {
            if (b < 64) {
                out_buf->keycode = keycodes_digits[b % MEW_DIGITS_COUNT];
            } else if ((b > 64) && (b < 128)) {
                out_buf->keycode = keycodes_symbols[b % MEW_SYMBOLS_COUNT];
            } else {
                out_buf->keycode = keycodes_letter[b % MEW_LETTERS_COUNT];
            }
            out_buf->mod = ((b % 2) == 1) ? MEW_MOD_NO : MEW_MOD_UPPERCASE;
        } else {
            if (b < 128) {
                out_buf->keycode = keycodes_digits[b % MEW_DIGITS_COUNT];
                out_buf->mod = MEW_MOD_NO;
            } else {
                out_buf->keycode = keycodes_letter[b % MEW_LETTERS_COUNT];
                out_buf->mod = ((b % 2) == 1) ? MEW_MOD_NO : MEW_MOD_UPPERCASE;
            }
        }
    } else {
        if (b < 96) {
            out_buf->keycode = keycodes_digits[b % MEW_DIGITS_COUNT];
        } else {
            out_buf->keycode = keycodes_letter[b % MEW_LETTERS_COUNT];
        }
        out_buf->mod = MEW_MOD_NO;
    }
}
