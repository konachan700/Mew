#include "debug.h"
#include "flash.h"
#include "../system/system.h"
#include "../../app/pwd_generator/mew_passgen.h"
#include "ui/ui.h"

static const uint8_t _mew_ss_array[sizeof(mew_unique_data)] = { [0 ... (sizeof(mew_unique_data)-1)] = 0xFF };
static mew_unique_data _mew_ss_temp;

const mew_unique_data *mew_keystore = (mew_unique_data*) &_mew_ss_array;

unsigned int mew_is_first_start(void) {
	return (mew_keystore->magic == MEW_MAGIC) ? 0 : 1;
}

unsigned int mew_otp_write_temporary_data(void) {
	uint32_t i;

    uint8_t* _mew_ss_temp_arr = (uint8_t *) &_mew_ss_temp;

	if (mew_keystore->magic == 0xFFFFFFFF) {
        mew_debug_print("Writing keys to flash...");

        flash_unlock();
        flash_program((uint32_t) &_mew_ss_array, _mew_ss_temp_arr, sizeof(mew_unique_data));
        flash_lock();

        memset((void*) &_mew_ss_temp, 0, sizeof(mew_unique_data));

        if (_mew_ss_temp.magic != 0) {
            mew_debug_die_with_message("mew_ss_init: insecure operation detected");
        } else {
            mew_debug_print("Temporary data was written.");
        }
	} else if (mew_keystore->magic == MEW_MAGIC) {
		// Do nothing
	} else {
		mew_debug_die_with_message("Inital flash data is broken.");
	}

	return 0;
}

unsigned int mew_otp_storage_init(void) {
    uint32_t i, crc_p, crc_m;

    uint32_t* mkey32_table = (uint32_t *) &_mew_ss_temp.master_key.table;
    uint32_t* pkey32_table = (uint32_t *) &_mew_ss_temp.passwords_key.table;
    
    uint8_t* mkey8_table = (uint8_t *) &_mew_ss_temp.master_key.table;
    uint8_t* pkey8_table = (uint8_t *) &_mew_ss_temp.passwords_key.table;
    
    if (mew_keystore->magic == MEW_MAGIC) {
        mkey32_table = (uint32_t *) &mew_keystore->master_key.table;
        pkey32_table = (uint32_t *) &mew_keystore->passwords_key.table;

        crc_reset();
        crc_m = crc_calculate_block(mkey32_table, (MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE) / 4);

        crc_reset();
        crc_p = crc_calculate_block(pkey32_table, (MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE) / 4);

        if ((crc_m != mew_keystore->master_key.crc32) || (crc_p != mew_keystore->passwords_key.crc32)) {
            mew_debug_die_with_message("mew_ss_init: bad crc");
        }
    } else {
    	mew_debug_print("Generating keys...");

        memset((void*) &_mew_ss_temp, 0, sizeof(mew_unique_data));
        _mew_ss_temp.magic = MEW_MAGIC;

        crc_reset();
        mew_xor_keygen(mkey8_table, MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE);
        _mew_ss_temp.master_key.crc32 = crc_calculate_block(mkey32_table, (MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE) / 4);

        crc_reset();
        mew_xor_keygen(pkey8_table, MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE);
        _mew_ss_temp.passwords_key.crc32 = crc_calculate_block(pkey32_table, (MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE) / 4);

        for (i=0; i<512; i++) {
            _mew_ss_temp.system_encrypt_iv[i % 2] ^= mew_random64();
            _mew_ss_temp.system_encrypt_key[i % 4] ^= mew_random64();
        }

        mew_qrcode_set_data((uint8_t*)_mew_ss_temp.system_encrypt_key, 0, 4 * sizeof(uint64_t));
        mew_qrcode_set_data((uint8_t*)_mew_ss_temp.system_encrypt_iv, 4 * sizeof(uint64_t), 2 * sizeof(uint64_t));
        //mew_ui_show_window(MEW_WINDOW_INITIAL_QR);

        mew_debug_print("Temporary keys was generated.");
    }
    
    return 0;
}
