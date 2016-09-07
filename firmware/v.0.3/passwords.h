#ifndef PASSWORDS_H_
#define PASSWORDS_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/flash.h>
#include <stdlib.h>
#include "usb.h"
#include "libatcrypto/cryptoauthlib.h"

#define MEW_PASSWORD_LEN 	32 // максимальная длина пароля
#define MEW_LOGIN_LEN 		64 // максимальная длина логина
#define MEW_RESERVED_LEN 	25 // длина пустого места в структуре (тут будет локальный конфиг)

#define MEW_CONFIG_ATSHA204A_DEV 		1 // означает версию MeW с установленной ATSHA204A

#define MEW_CONFIG_ENTER_ENABLE 		1
#define MEW_CONFIG_ENTER_DISABLE 		0

// Определяет, отдается логин вместе с паролем или нет. Это глобальное поле, переопределяющее локальный конфиг.
// Если функция отключена этой константой, включить локально ее будет нельзя.
#define MEW_CONFIG_LOGIN_NOT_USED		0 // Запретить отдачу логинов
#define MEW_CONFIG_LODIN_USED_TAB		1 // Логин отдается перед паролем через TAB
#define MEW_CONFIG_LOGIN_USED_ENTER 	2 // Логин отдается перед паролем через ENTER

// Пин-код, используется или нет.
// Пин-код тут устроен интересно - он не блокирует доступ к устройству, но искажает пароли, если введен неверно.
// Это очень осложняет перебор пин-кода.
#define MEW_CONFIG_PIN_NOT_SET			0 // Пин-код не установлен
#define MEW_CONFIG_PIN_SET				1 // Пин-код установлен

// Версия конфига. Важно только для управляющей программы.
#define MEW_CONFIG_VERSION				0x02

struct __attribute__((__packed__)) mew_config {
	volatile uint32_t signature;
	volatile uint32_t version;
	volatile uint8_t use_pin;
	volatile uint8_t use_login;
	volatile uint8_t press_enter;
	volatile uint8_t i2c_dev_present;
	volatile uint32_t last_config_time;
};

struct __attribute__((__packed__)) password_record {
	volatile unsigned char header[2];
	volatile unsigned char login[MEW_LOGIN_LEN];
	volatile unsigned char password[MEW_PASSWORD_LEN];
	volatile unsigned char bank_id;
	volatile unsigned char login_len;
	volatile unsigned char password_len;
	volatile unsigned char uppercase_enable;
	volatile unsigned char enter_enable;
	volatile unsigned char reserved[MEW_RESERVED_LEN];
};

#define FLASH_SIZE 							64
#define FLASH_PAGE_SIZE 					1024
#define FLASH_DATA_SIZE 					4
#define FLASH_PASSWORD_STORE_GLOBAL_OFFSET 	((FLASH_BASE + (FLASH_PAGE_SIZE * FLASH_SIZE)) - (FLASH_PAGE_SIZE * FLASH_DATA_SIZE))
#define FLASH_PASSWORD_STORE_LOCAL_OFFSET	1
#define FLASH_PASSWORD_STORE_SIGN_OFFSET	0
#define FLASH_PASSWORD_RECORD_LEN			(sizeof(struct password_record))
#define FLASH_PASSWORD_STORE_SIZE			(((FLASH_PAGE_SIZE * FLASH_DATA_SIZE) / FLASH_PASSWORD_RECORD_LEN) - FLASH_PASSWORD_STORE_LOCAL_OFFSET)
#define PASSWORD_STORE_SIGNATURE			0x437043EA
#define PASSWORDS_IN_BANK					4

extern void mew_inital_fill_passwords(void);
extern void mew_flash_write_struct(uint32_t addr, void *buf, uint32_t size);
extern struct mew_config* mew_get_config(void);
extern struct password_record* mew_get_password_record(uint8_t bank, uint8_t button) ;
extern void mew_password_send(uint8_t bank, uint8_t button);

#endif /* PASSWORDS_H_ */
