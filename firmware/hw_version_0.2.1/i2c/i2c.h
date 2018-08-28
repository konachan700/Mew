#ifndef MEW_I2C_H
#define MEW_I2C_H

#include "../config.h"

#define MEW_I2C_TS_I2C                  I2C3
#define MEW_I2C_TS_NVIC                 NVIC_I2C3_EV_IRQ
#define MEW_I2C_TS_ISR                  i2c1_ev_isr

#define MEW_I2C_TS_SCL_PORT             GPIOA
#define MEW_I2C_TS_SCL_PIN              GPIO8
#define MEW_I2C_TS_SCL_AF               GPIO_AF4

#define MEW_I2C_TS_SDA_PORT             GPIOC
#define MEW_I2C_TS_SDA_PIN              GPIO9
#define MEW_I2C_TS_SDA_AF               GPIO_AF4

#define MEW_I2C_MODE_0BIT               2
#define MEW_I2C_MODE_8BIT               0
#define MEW_I2C_MODE_16BIT              1

void mew_i2c_init(void);
uint8_t mew_i2c_read(uint8_t dev_addr, uint16_t data_addr, uint8_t* data, uint8_t mode);
uint8_t mew_i2c_write(uint8_t dev_addr, uint16_t data_addr, uint8_t data, uint8_t mode);

uint8_t mew_i2c_read_block_ts2007(uint8_t dev_addr, uint8_t cmd, uint8_t* data, uint8_t size);

#endif /* I2C_H */

