/*
 * sw_i2c.h
 *
 *  Created on: Aug 26, 2016
 *      Author: misaki
 */

#ifndef LIBATCRYPTO_HAL_SW_I2C_H_
#define LIBATCRYPTO_HAL_SW_I2C_H_

#include <libopencm3/stm32/gpio.h>
#include "delay.h"

#define I2C_SDA_PORT GPIOB
#define I2C_SCL_PORT GPIOB

#define MEW_I2C_SDA_PIN_L 	(gpio_get(I2C_SDA_PORT, GPIO_I2C1_SDA) == 0)
#define MEW_I2C_SDA_PIN_H 	(gpio_get(I2C_SDA_PORT, GPIO_I2C1_SDA) != 0)

#define MEW_I2C_SDA_L 		gpio_clear(I2C_SDA_PORT, GPIO_I2C1_SDA)
#define MEW_I2C_SDA_H 		gpio_set(I2C_SDA_PORT, GPIO_I2C1_SDA)

#define MEW_I2C_SCL_L 		gpio_clear(I2C_SCL_PORT, GPIO_I2C1_SCL)
#define MEW_I2C_SCL_H 		gpio_set(I2C_SCL_PORT, GPIO_I2C1_SCL)

#define MEW_DELAY			delay_us(1, 72000000L)

enum MEW_I2C_RETVAL {
	OK,
	BUS_BUSY,
	BUS_BROKEN,
	GENERAL_WRITE_ERROR
};

extern void SWI2C_init(void);
extern enum MEW_I2C_RETVAL SWI2C_read_bytes(uint8_t da, uint8_t *buf, uint8_t length);
extern enum MEW_I2C_RETVAL SWI2C_write_bytes(uint8_t da, uint8_t *buf, uint8_t length);

#endif /* LIBATCRYPTO_HAL_SW_I2C_H_ */
