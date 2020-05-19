#include "debug.h"
#include "../i2c/i2c.h"
#include "crypto.h"
#include "../system/system.h"
#include "../flash/flash.h"

mew_secure_settings mew_ss;
extern const mew_unique_data *mew_keystore;

#define MEW_EEP_DATA_OFFSET_8 	16
#define MEW_EEP_DATA_SIZE_8		(MEW_I2C_EEPROM_PAGE_SIZE - MEW_EEP_DATA_OFFSET_8)

#define MEW_EEP_SIZE_32 		(MEW_I2C_EEPROM_PAGE_SIZE / sizeof(uint32_t))
#define MEW_EEP_DATA_SIZE_32 	(MEW_EEP_DATA_SIZE_8 / sizeof(uint32_t))
#define MEW_EEP_DATA_OFFSET_32 	(MEW_EEP_DATA_OFFSET_8 / sizeof(uint32_t))

static uint32_t data_out_32[MEW_EEP_SIZE_32];
static uint8_t* data_out_8 = (uint8_t*) data_out_32; // MEW_I2C_EEPROM_PAGE_SIZE

static uint32_t data_decrypted_32[MEW_EEP_DATA_SIZE_32];
static uint8_t* data_decrypted_8 = (uint8_t*) data_decrypted_32;

static uint8_t data_sign[8];
static uint8_t data_key_sign[8];
static uint8_t data_temp_01[40];

unsigned int mew_load_secure_settings(void) {
	uint32_t i;

	mew_i2c_eeprom_read(0, data_out_8);
	//mew_debug_print_hex_ex("Read data: ", (const char *) data_out_8, MEW_I2C_EEPROM_PAGE_SIZE);

	mew_hash8(data_out_8 + MEW_EEP_DATA_OFFSET_8, MEW_EEP_DATA_SIZE_8, data_sign);
	//mew_debug_print_hex_ex("Calc sign: ", (const char *) data_sign, 8);

	memcpy(data_temp_01, data_sign, 8);
	memcpy(data_temp_01 + 8, mew_keystore->system_encrypt_key, 32);
	mew_hash8(data_temp_01, 40, data_key_sign);

	if (memcmp(data_key_sign, data_out_8, 8) == 0) {
		mewcrypt_aes256(MEW_DECRYPT,
				data_out_32 + MEW_EEP_DATA_OFFSET_32,
				data_decrypted_32,
				MEW_EEP_DATA_SIZE_32,
				mew_keystore->system_encrypt_key,
				mew_keystore->system_encrypt_iv);

		memcpy(&mew_ss, data_decrypted_8, sizeof(mew_secure_settings));

		mew_ss.is_valid = 1;
		mew_debug_print("mew_load_secure_settings: ok");
	} else {
		mew_ss.is_valid = 0;
		mew_debug_print("mew_load_secure_settings: data corrupted or new device?");
	}
	return 0;
}

void mew_save_secure_settings(void) {
	mew_debug_print("mew_save_secure_settings: saving...");

	memset(data_decrypted_8, 0, MEW_EEP_DATA_SIZE_8);
	memcpy(data_decrypted_8, &mew_ss, sizeof(mew_secure_settings));
	memset(data_out_8, 0, MEW_I2C_EEPROM_PAGE_SIZE);

	mewcrypt_aes256(MEW_ENCRYPT,
			data_decrypted_32,
			data_out_32 + MEW_EEP_DATA_OFFSET_32,
			MEW_EEP_DATA_SIZE_32,
			mew_keystore->system_encrypt_key,
			mew_keystore->system_encrypt_iv);

	mew_hash8(data_out_8 + MEW_EEP_DATA_OFFSET_8, MEW_EEP_DATA_SIZE_8, data_sign);

	memcpy(data_temp_01, data_sign, 8);
	memcpy(data_temp_01 + 8, mew_keystore->system_encrypt_key, 32);
	mew_hash8(data_temp_01, 40, data_key_sign);

	memcpy(data_out_8, data_key_sign, 8);

	//mew_debug_print_hex_ex("Write data: \r\n", (const char *) data_out_8, MEW_I2C_EEPROM_PAGE_SIZE);
	//mew_debug_print_hex_ex("Calc sign: \r\n", (const char *) data_key_sign, 8);

	mew_i2c_eeprom_write(0, data_out_8);

	mew_debug_print("mew_save_secure_settings: saving ok");
}


