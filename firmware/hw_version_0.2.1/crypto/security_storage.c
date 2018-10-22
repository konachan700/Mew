#include "../debug/debug.h"
#include "crypto.h"

static const uint8_t _mew_ss_array[sizeof(mew_unique_data)] = { [0 ... (sizeof(mew_unique_data)-1)] = 0xFF };
static const mew_unique_data *_mew_ss = (mew_unique_data*) &_mew_ss_array;

void mew_ss_init(void) {
    uint32_t i;
    
    uint32_t crc_p, crc_m;
    mew_unique_data _mew_ss_temp;
    uint8_t* _mew_ss_temp_arr = (uint8_t *) &_mew_ss_temp;
    
    uint32_t* mkey32_table = (uint32_t *) &_mew_ss_temp.master_key.table;
    uint32_t* pkey32_table = (uint32_t *) &_mew_ss_temp.passwords_key.table;
    
    uint8_t* mkey8_table = (uint8_t *) &_mew_ss_temp.master_key.table;
    uint8_t* pkey8_table = (uint8_t *) &_mew_ss_temp.passwords_key.table;
    
   if (_mew_ss->magic != MEW_MAGIC) {
       mew_debug_print("Generating keys...");
       
       flash_unlock();
       crc_reset();
       
       memset((void*) &_mew_ss_temp, 0, sizeof(mew_unique_data));
       _mew_ss_temp.magic = MEW_MAGIC;
       
       mew_xor_keygen(mkey8_table, MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE);
       _mew_ss_temp.master_key.crc32 = crc_calculate_block(mkey32_table, (MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE) / 4);
       
       mew_xor_keygen(pkey8_table, MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE);
       _mew_ss_temp.passwords_key.crc32 = crc_calculate_block(pkey32_table, (MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE) / 4);
       
       for (i=0; i<512; i++) {
           _mew_ss_temp.system_encrypt_iv[i % 2] ^= mew_random64();
           _mew_ss_temp.system_encrypt_key[i % 4] ^= mew_random64();
       }
       //mew_debug_print_hex((uint8_t *)&_mew_ss_temp, sizeof(mew_unique_data));
       
       flash_program((uint32_t) &_mew_ss_array, _mew_ss_temp_arr, sizeof(mew_unique_data));
       
       flash_lock();
       memset((void*) &_mew_ss_temp, 0, sizeof(mew_unique_data));
       if (_mew_ss_temp.magic != 0) {
           mew_die_with_message("mew_ss_init: insecure operation detected");
       } else {
           mew_debug_print("First time data are written");
       }
   } else {
       mkey32_table = (uint32_t *) &_mew_ss->master_key.table;
       pkey32_table = (uint32_t *) &_mew_ss->passwords_key.table;
       
       crc_m = crc_calculate_block(mkey32_table, (MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE) / 4);
       crc_p = crc_calculate_block(pkey32_table, (MEW_MAGIC_TSIZE * MEW_MAGIC_TSIZE) / 4);
       
       if ((crc_m != _mew_ss->master_key.crc32) || (crc_p != _mew_ss->passwords_key.crc32)) {
           mew_die_with_message("mew_ss_init: bad crc");
       }
   }
}