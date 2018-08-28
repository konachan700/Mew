#include "debug.h"
#include "../usb/mew_usb_hid.h"
#include "../flash/flash.h"
#include "../bluetooth/bluetooth.h"
#include "../crypto/crypto.h"
#include "../display/display.h"
#include "../i2c/i2c.h"

#ifdef INTEGRATION_TESTS
extern volatile uint8_t mew_usb_init_state;
uint32_t data_in[64];
uint32_t data_out[64];
#endif

#ifdef INTEGRATION_TESTS
void mew_integration_test(void) {
    uint32_t i;
    
    mew_debug_print("\n**********************************");
    mew_debug_print("* RUNNING TESTS");
    mew_debug_print("**********************************");
    
    mew_debug_print("Usart OK");
    mew_debug_print("Clocks OK");
    if (mew_get_millis() > 0) {
        mew_debug_print("Systick OK");
    }
    if (mew_usb_init_state > 0) {
        mew_debug_print("USB OK");
    }
    
    mew_debug_print("Flash id reading...");
    uint8_t id[3];
    mew_flash_get_id(id);
    mew_debug_print_hex((uint8_t*)id, 3);
    if (id[0] == 0xEF && id[1] == 0xAA && id[2] == 0x21) {
        mew_debug_print("Flash test OK");
    } else {
        mew_debug_print("Flash test failed.");
    }
    
    mew_debug_print("Encrypt pattern 0x07...");
    uint64_t data_key[4];
    uint64_t data_iv[2];
    memset(data_iv, 0xFF, sizeof(uint32_t) * 4);
    memset(data_key, 0x77, sizeof(uint32_t) * 8);
    memset(data_in, 0x07, sizeof(uint32_t) * 64);
    memset(data_out, 0x00, sizeof(uint32_t) * 64);
    mewcrypt_aes256(MEW_ENCRYPT, data_in, data_out, 16, data_key, data_iv);
    mew_debug_print_hex((uint8_t *)data_out, sizeof(uint32_t) * 16);
    mew_debug_print("Decrypt pattern 0x07...");
    memset(data_in, 0x07, sizeof(uint32_t) * 64);
    mewcrypt_aes256(MEW_DECRYPT, data_out, data_in, 16, data_key, data_iv);
    mew_debug_print_hex((uint8_t *)data_in, sizeof(uint32_t) * 16);
    for(i=0; i<16; i++) {
        if (data_in[i] != 0x07070707) {
            i = 9999;
            mew_debug_print("AES-256 Error");
            break;
        }
    }
    if (i != 9999) mew_debug_print("AES-256 OK");
    
    mew_debug_print("Random testing...");
    for(i=0; i<4; i++) {
        data_in[i] = mew_random32();
    }
    mew_debug_print_hex((uint8_t *)data_in, sizeof(uint32_t) * 4);
    mew_debug_print("Random OK");

    mew_debug_print("Hash algo testing...");
    memset(data_in, 0x54, sizeof(uint32_t) * 64);
    memset(data_out, 0x00, sizeof(uint32_t) * 64);
    mew_hash32(data_in, 16, data_out);
    mew_debug_print_hex((uint8_t *)data_out, sizeof(uint32_t) * 8);
    mew_debug_print("Hash OK");
    
    uint8_t data_i2c = 0x77;
    uint8_t res_i2c = mew_i2c_write(0x50, 0x00, data_i2c, MEW_I2C_MODE_16BIT);
    if (res_i2c == 1) {
        mew_debug_print("I2C write test ok");
    }
    data_i2c = 0;
    
    res_i2c = mew_i2c_read(0x50, 0x00, &data_i2c, MEW_I2C_MODE_16BIT);
    if (res_i2c == 1 && data_i2c == 0x77) {
        mew_debug_print("I2C read test ok");
    }
    
    
    mew_debug_print("Test block ended");
    mew_debug_print("**********************************");
}
#endif