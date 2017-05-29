#ifndef PASSWORDS_H_
#define PASSWORDS_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/flash.h>

struct __attribute__((__packed__)) password_record {
	unsigned char password[31];
	unsigned char bank_id;
};

#define FLASH_PASSWORD_STORE_GLOBAL_OFFSET 	0x0801FC00
#define FLASH_PASSWORD_STORE_RECORD_LEN		64
#define FLASH_PASSWORD_STORE_LOCAL_OFFSET	1
#define FLASH_PASSWORD_STORE_SIGN_OFFSET	0

#define PASSWORD_STORE_SIGNATURE 0x0F0430043

#endif /* PASSWORDS_H_ */
