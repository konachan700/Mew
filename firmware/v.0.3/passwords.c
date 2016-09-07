#include "passwords.h"

unsigned char mew_get_random_byte(void);
void mew_gen_password_record(struct password_record *pr);

void mew_inital_fill_passwords(void) {
	unsigned int i;
	struct password_record pr;
	struct mew_config conf;

	uint32_t sign = MMIO32(FLASH_PASSWORD_STORE_GLOBAL_OFFSET);
	if (sign == PASSWORD_STORE_SIGNATURE) return;

	flash_unlock();
	for (i=0; i<FLASH_DATA_SIZE; i++) {
		flash_erase_page(FLASH_PASSWORD_STORE_GLOBAL_OFFSET + (FLASH_PAGE_SIZE * i));
	}

	conf.signature 			= PASSWORD_STORE_SIGNATURE;
	conf.version 			= MEW_CONFIG_VERSION;
	conf.use_pin 			= MEW_CONFIG_PIN_NOT_SET;
	conf.use_login 			= MEW_CONFIG_LOGIN_NOT_USED;
	conf.i2c_dev_present 	= MEW_CONFIG_ATSHA204A_DEV;
	conf.press_enter		= MEW_CONFIG_ENTER_ENABLE;
	conf.last_config_time	= 0;

	mew_flash_write_struct(FLASH_PASSWORD_STORE_GLOBAL_OFFSET, &conf, sizeof(struct mew_config));

	for (i=FLASH_PASSWORD_STORE_LOCAL_OFFSET; i<FLASH_PASSWORD_STORE_SIZE; i++) {
		mew_gen_password_record(&pr);
		pr.header[0] = 0x43;
		pr.header[1] = i;
		pr.bank_id = ((i - FLASH_PASSWORD_STORE_LOCAL_OFFSET) / PASSWORDS_IN_BANK);
		pr.enter_enable = 1;

		/* Этот блок сделан для демонстрации разных режимов работы MeW */
		if (pr.bank_id == 0)
			pr.uppercase_enable = 0;
		else
			pr.uppercase_enable = 1;

		if (pr.bank_id == 3) {
			pr.uppercase_enable = 0;
			pr.password_len = 16;
		}

		mew_flash_write_struct(FLASH_PASSWORD_STORE_GLOBAL_OFFSET + (i * FLASH_PASSWORD_RECORD_LEN), &pr, FLASH_PASSWORD_RECORD_LEN);
	}

	flash_lock();
}

struct password_record* mew_get_password_record(uint8_t bank, uint8_t button) {
	uint32_t bank_offset = ((bank * PASSWORDS_IN_BANK) + button + FLASH_PASSWORD_STORE_LOCAL_OFFSET) * FLASH_PASSWORD_RECORD_LEN;
	return (struct password_record*)(FLASH_PASSWORD_STORE_GLOBAL_OFFSET + bank_offset);
}

struct mew_config* mew_get_config(void) {
	return (struct mew_config*)(FLASH_PASSWORD_STORE_GLOBAL_OFFSET);
}

void mew_password_send(uint8_t bank, uint8_t button) {
	uint32_t i;
	uint8_t curr_char, uppercase;
	struct password_record* pr = mew_get_password_record(bank, button);
	if (pr->header[0] == 0x43) {
		for (i=0; i<pr->password_len; i++) {
			curr_char = pr->password[i] & 0x7F;
			if (pr->uppercase_enable == 1)
				uppercase = (pr->password[i] & (1 << 7)) ? MEW_UPPER_CASE : MEW_LOWER_CASE;
			else
				uppercase = MEW_LOWER_CASE;
			mew_send_char(curr_char, uppercase);
		}

		if (pr->enter_enable == 1) {
			mew_send_char(0x28, MEW_LOWER_CASE);
		}
	}
}

void mew_flash_write_struct(uint32_t addr, void *buf, uint32_t size) {
	uint32_t i;
	for(i=0; i<size; i=i+2) {
		flash_program_half_word(addr+i, MMIO16((uint32_t)(buf)+i));
	}
}

void mew_gen_password_record(struct password_record *pr) {
	unsigned int i;

	pr->login_len = 0;
	for (i=0; i<MEW_LOGIN_LEN; i++) pr->login[i] = 0x00;

	pr->password_len = 32;
	for (i=0; i<MEW_PASSWORD_LEN; i++) pr->password[i] = mew_get_random_byte();

	for (i=0; i<MEW_RESERVED_LEN; i++) pr->reserved[i] = 0x00;
}

#ifdef ATSHA204A_PRESENT

#else
unsigned char mew_get_random_byte(void) {
	unsigned char random_byte, uppercase;
	while (1) {
		uppercase = (rand() & (1 << 7));
		random_byte = 0xFF & (rand() ^ DESIG_UNIQUE_ID0 ^ DESIG_UNIQUE_ID1 ^ DESIG_UNIQUE_ID2);
		if ((random_byte > 0x03) && (random_byte < 0x28)) return random_byte | uppercase;
	}
}
#endif
