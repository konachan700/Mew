#include "passwords.h"


void inital_fill_passwords(void) {
	unsigned long sign_offset = FLASH_PASSWORD_STORE_GLOBAL_OFFSET + (FLASH_PASSWORD_STORE_RECORD_LEN * FLASH_PASSWORD_STORE_SIGN_OFFSET);

	flash_unlock();
	flash_erase_page(FLASH_PASSWORD_STORE_GLOBAL_OFFSET);
	flash_program_word(sign_offset, PASSWORD_STORE_SIGNATURE);



}

void mew_gen_password_record(struct password_record* pr, unsigned char bank_id) {


}
