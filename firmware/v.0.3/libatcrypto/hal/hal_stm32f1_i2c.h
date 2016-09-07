/*
 * hal_stm32f1_i2c.h
 *
 *  Created on: Aug 25, 2016
 *      Author: misaki
 */

#ifndef LIBATCRYPTO_HAL_HAL_STM32F1_I2C_H_
#define LIBATCRYPTO_HAL_HAL_STM32F1_I2C_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "atca_hal.h"
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>
//#include "sw_i2c.h"
#include "mew_i2c.h"

#define MEW_ATSHA204A_I2C_ADDR 0xC8

void change_i2c_speed(ATCAIface iface, uint32_t speed);

#endif /* LIBATCRYPTO_HAL_HAL_STM32F1_I2C_H_ */
