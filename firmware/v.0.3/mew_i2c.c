/*
 * i2c.c
 *
 *  Created on: Aug 29, 2016
 *      Author: misaki
 */

#include "mew_i2c.h"

volatile uint8_t device_address = 0x00;

static inline void __disable_irq(void)  { asm volatile("cpsid i"); }
static inline void __enable_irq(void)   { asm volatile("cpsie i"); }

void mew_i2c1_init() {
	rcc_periph_clock_enable(RCC_I2C1);
	rcc_periph_clock_enable(RCC_AFIO);

	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, GPIO_I2C1_SCL | GPIO_I2C1_SDA);
	rcc_periph_reset_pulse(RST_I2C1);

	i2c_peripheral_disable(I2C1);
	i2c_set_clock_frequency(I2C1, I2C_CR2_FREQ_36MHZ);
	i2c_set_fast_mode(I2C1);
	i2c_set_ccr(I2C1, 0x1e);
	i2c_set_trise(I2C1, 0x0b);
	i2c_set_own_7bit_slave_address(I2C1, 0x00);
	i2c_peripheral_enable(I2C1);
}

enum MEW_I2C_RETVAL mew_i2c_write(uint32_t i2c_int, uint8_t da, uint8_t *buf, uint32_t buf_len) {
    volatile uint32_t __attribute__((unused)) temp = 0;
    volatile uint32_t Timeout = 0;

    I2C_CR2(i2c_int) |= I2C_CR2_ITERREN;
    Timeout = 0xFFFF;
    I2C_CR1(i2c_int) |= 0x0100;
    while ((I2C_SR1(i2c_int) & 0x0001) != 0x0001) {
        if (Timeout-- == 0) return GENERAL_ERROR_WRITE;
    }

    da &= 0xFFFE;
    device_address = da;
    I2C_DR(i2c_int) = device_address;

    Timeout = 0xFFFF;
    while ((I2C_SR1(i2c_int) & 0x0002) != 0x0002) {
        if (Timeout-- == 0) return GENERAL_ERROR_WRITE;
    }

    temp = I2C_SR2(i2c_int);
    I2C_DR(i2c_int) = *buf;
    buf++;
    buf_len--;

    while (buf_len--) {
        while ((I2C_SR1(i2c_int) & 0x00004) != 0x000004);
        I2C_DR(i2c_int) = *buf;
        buf++;
    }

    while ((I2C_SR1(i2c_int) & 0x00004) != 0x000004);
    I2C_CR1(i2c_int) |= 0x0200;
    while ((I2C_CR1(i2c_int) & 0x200) == 0x200);

	return OK;
}

enum MEW_I2C_RETVAL mew_i2c_read(uint32_t i2c_int, uint8_t da, uint8_t *buf, uint32_t buf_len) {
    volatile uint32_t __attribute__((unused)) temp = 0;
    volatile uint32_t Timeout = 0;

    I2C_CR2(i2c_int) |= I2C_CR2_ITERREN;

    if (buf_len == 1) {
        Timeout = 0xFFFF;
        I2C_CR1(i2c_int) |= 0x0100;
        while ((I2C_SR1(i2c_int) & 0x0001) != 0x0001) {
            if (Timeout-- == 0) return GENERAL_ERROR_READ;
        }

        da |= 0x0001;
        device_address = da;
        I2C_DR(i2c_int) = device_address;

        Timeout = 0xFFFF;
        while ((I2C_SR1(i2c_int) & 0x0002) != 0x0002) {
            if (Timeout-- == 0) return GENERAL_ERROR_READ;
        }

        I2C_CR1(i2c_int) &= 0xFBFF;
        __disable_irq();
        temp = I2C_SR2(i2c_int);
        I2C_CR1(i2c_int) |= 0x0200;
        __enable_irq();
        while ((I2C_SR1(i2c_int) & 0x00040) != 0x000040);
        *buf = I2C_DR(i2c_int);
        while ((I2C_SR1(i2c_int) & 0x200) == 0x200);
        I2C_CR1(i2c_int) |= 0x0400;
    } else if (buf_len == 2) {
    	I2C_CR1(i2c_int) |= 0x0800;
        I2C_CR1(i2c_int) |= 0x0100;

        Timeout = 0xFFFF;
        while ((I2C_SR1(i2c_int) & 0x0001) != 0x0001) {
            if (Timeout-- == 0) return GENERAL_ERROR_READ;
        }

        da |= 0x0001;
        device_address = da;
        I2C_DR(i2c_int) = device_address;

        Timeout = 0xFFFF;
        while ((I2C_SR1(i2c_int) & 0x0002) != 0x0002) {
            if (Timeout-- == 0) return GENERAL_ERROR_READ;
        }

        __disable_irq();
        temp = I2C_SR2(i2c_int);
        I2C_CR1(i2c_int) &= 0x0400;
        __enable_irq();
        while ((I2C_SR1(i2c_int) & 0x00004) != 0x000004);
        __disable_irq();
        I2C_CR1(i2c_int) &= 0xFDFF;
        *buf = I2C_DR(i2c_int);
        __enable_irq();
        buf++;
        *buf = I2C_DR(i2c_int);
        while ((I2C_CR1(i2c_int) & 0x200) == 0x200);
        I2C_CR1(i2c_int)  |= 0x0400;
        I2C_CR1(i2c_int)  &= 0xF7FF;
    } else {
        I2C_CR1(i2c_int) |= 0x0100;
        Timeout = 0xFFFF;
        while ((I2C_SR1(i2c_int) & 0x0001) != 0x0001) {
            if (Timeout-- == 0) return GENERAL_ERROR_READ;
        }

        da |= 0x0001;
        device_address = da;
        I2C_DR(i2c_int) = device_address;

        Timeout = 0xFFFF;
        while ((I2C_SR1(i2c_int) & 0x0002) != 0x0002) {
            if (Timeout-- == 0) return GENERAL_ERROR_READ;
        }

        temp = I2C_SR2(i2c_int);

        while (buf_len) {
            if (buf_len != 3) {
                while ((I2C_SR1(i2c_int) & 0x00004) != 0x000004);
                *buf = I2C_DR(i2c_int);
                buf++;
                buf_len--;
            }

            if (buf_len == 3) {
                while ((I2C_SR1(i2c_int) & 0x00004) != 0x000004);
                I2C_CR1(i2c_int) &= 0x0400;
                __disable_irq();
                *buf = I2C_DR(i2c_int);
                buf++;
                I2C_CR1(i2c_int) |= 0x0200;
                *buf = I2C_DR(i2c_int);
                __enable_irq();
                buf++;
                while ((I2C_SR1(i2c_int) & 0x00040) != 0x000040);
                *buf = I2C_DR(i2c_int);
                buf_len = 0;

            }
        }
        while ((I2C_CR1(i2c_int) & 0x200) == 0x200);
        I2C_CR1(i2c_int) |= 0x0400;
    }
	return OK;
}
