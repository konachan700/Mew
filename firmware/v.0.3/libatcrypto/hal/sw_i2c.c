/*
 * sw_i2c.c
 *
 *      Написано только потому, что errata полторы страницы, да и реализация такая объемная судя по коду от CubeMX,
 *      что проще запилить ногодрыг - разницы нет, в обоих случаях операция блокирующая.
 *      Тащить второй HAL ради одного i2c нужным не считаю. А в libopencm3 реализованы только обертки-примитивы для работы с i2c,
 *      то есть код для полноценной работы будет тоже огромный.
 *      Впоследствии, может быть, перепишу по-людски, с DMA и прерываниями, сейчас и так сойдет.
 *
 */

#include "sw_i2c.h"

enum MEW_I2C_RETVAL __sw_i2c_start();
void __sw_i2c_stop();
void __sw_i2c_ack();
void __sw_i2c_no_ack();
bool __sw_i2c_is_ack();
void __sw_i2c_write_byte(uint8_t b);
uint8_t __sw_i2c_read_byte();

void SWI2C_init(void) {
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_OPENDRAIN, GPIO_I2C1_SCL | GPIO_I2C1_SDA);
	gpio_set(GPIOB, GPIO_I2C1_SCL | GPIO_I2C1_SDA);
}

enum MEW_I2C_RETVAL SWI2C_read_bytes(uint8_t da, uint8_t *buf, uint8_t length) {
	enum MEW_I2C_RETVAL start_val;

	start_val = __sw_i2c_start();
	if (start_val != OK) return start_val;

	__sw_i2c_write_byte(da + 1);

	if (!__sw_i2c_is_ack()) {
		__sw_i2c_stop();
		return GENERAL_WRITE_ERROR;
	}

	while (length != 0) {
		*buf = __sw_i2c_read_byte();

		buf++;
		length--;

		if (length > 0) {
			__sw_i2c_ack();
		} else {
			__sw_i2c_no_ack();
			break;
		}
	}
	__sw_i2c_stop();
	return OK;
}

enum MEW_I2C_RETVAL SWI2C_write_bytes(uint8_t da, uint8_t *buf, uint8_t length) {
	enum MEW_I2C_RETVAL start_val;

	start_val = __sw_i2c_start();
	if (start_val != OK) return start_val;

	__sw_i2c_write_byte(da);

	if (!__sw_i2c_is_ack()) {
		__sw_i2c_stop();
		return GENERAL_WRITE_ERROR;
	}

	while (length != 0) {
		__sw_i2c_write_byte(*buf);

		if (!__sw_i2c_is_ack()) {
			__sw_i2c_stop();
			return GENERAL_WRITE_ERROR;
		}

		buf++;
		length--;
	}

	__sw_i2c_stop();
	return OK;
}

enum MEW_I2C_RETVAL __sw_i2c_start() {
	MEW_I2C_SCL_H;
	MEW_DELAY;
	MEW_I2C_SDA_L;
	MEW_DELAY;

	//if (MEW_I2C_SDA_PIN_L) return BUS_BUSY;

	MEW_I2C_SCL_L;
	MEW_DELAY;

	//if (MEW_I2C_SDA_PIN_H) return BUS_BROKEN;

	return OK;
}

void __sw_i2c_stop() {
	MEW_I2C_SDA_L;
	MEW_I2C_SCL_L;
	MEW_DELAY;
	MEW_I2C_SCL_H;
	MEW_DELAY;
	MEW_I2C_SDA_H;
}

void __sw_i2c_ack() {
	MEW_I2C_SCL_L;
	MEW_DELAY;
	MEW_I2C_SDA_L;
	MEW_DELAY;
	MEW_I2C_SCL_H;
	MEW_DELAY;
	MEW_I2C_SCL_L;
	MEW_DELAY;
}

void __sw_i2c_no_ack() {
	MEW_I2C_SCL_L;
	MEW_DELAY;
	MEW_I2C_SDA_H;
	MEW_DELAY;
	MEW_I2C_SCL_H;
	MEW_DELAY;
	MEW_I2C_SCL_L;
	MEW_DELAY;
}

bool __sw_i2c_is_ack() {
	MEW_I2C_SCL_L;
	MEW_DELAY;
	MEW_I2C_SDA_H;
	MEW_DELAY;
	MEW_I2C_SCL_H;
	MEW_DELAY;

	if (MEW_I2C_SDA_PIN_L) {
		MEW_I2C_SCL_L;
		MEW_DELAY;
		return false;
	}

	MEW_I2C_SCL_L;
	MEW_DELAY;

	return true;
}

void __sw_i2c_write_byte(uint8_t b) {
	uint8_t i = 0x08;
	while (i--) {
		MEW_I2C_SCL_L;
		MEW_DELAY;

		if (b & 0x80)MEW_I2C_SDA_H; else MEW_I2C_SDA_L;
		b <<= 1;
		MEW_DELAY;

		MEW_I2C_SCL_H;
		MEW_DELAY;
	}

	MEW_I2C_SCL_L;
	MEW_DELAY;
}

uint8_t __sw_i2c_read_byte() {
	uint8_t i = 8, b = 0;

	MEW_I2C_SDA_H;
	while (i--) {
		b <<= 1;

		MEW_I2C_SCL_L;
		MEW_DELAY;

		MEW_I2C_SCL_H;
		MEW_DELAY;

		if (MEW_I2C_SDA_PIN_H) b |= 0x01;
	}

	MEW_I2C_SCL_L;
	MEW_DELAY;

	return b;
}
