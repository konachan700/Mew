/*
 * i2c.h
 *
 *  Created on: Aug 29, 2016
 *      Author: misaki
 */

#ifndef I2C_H_
#define I2C_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>

typedef enum MEW_I2C_RETVAL {
	OK,
	GENERAL_ERROR_READ,
	GENERAL_ERROR_WRITE
};

extern void mew_i2c1_init();
extern enum MEW_I2C_RETVAL mew_i2c_read(uint32_t i2c_int, uint8_t da, uint8_t *buf, uint32_t buf_len);
extern enum MEW_I2C_RETVAL mew_i2c_write(uint32_t i2c_int, uint8_t da, uint8_t *buf, uint32_t buf_len);

#endif /* I2C_H_ */
