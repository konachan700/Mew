#ifndef __BOARD_MEW_BSP__
#define __BOARD_MEW_BSP__

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/assert.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/f2/rng.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/i2c.h>

typedef unsigned char u8;
typedef unsigned int  u16;
typedef unsigned long u32;

#define SPI_RX_DUMMY_BUF_SIZE 16
#define I2C_TIMEOUT (1024 * 1024)

#define EEPROM_READ  1
#define EEPROM_WRITE 2

struct i2c_eeprom_transaction {
    u8  type;
    u8  device_addr;
    u16 rw_addr;
    u16 count;
    u8* buffer;
    u8  state;
    u8  error;
    void (*on_rw_ok)(struct i2c_eeprom_transaction* me);
    void (*on_error)(struct i2c_eeprom_transaction* me);
}

#include "ui.h"
#include "ILI9341.h"
#include "menu.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int _write(int file, char *ptr, int len);

extern void start_debug_usart(void);
extern void start_all_clock(void);
extern void start_timer_2(void);
extern void start_backlight(void);
extern void start_buttons(void);
extern void start_leds(void);

extern void start_i2c1(void);
extern u32 mew_i2c_read(u8 da, u8 ra);

extern void start_spi_2_dma(void);
extern void start_spi_2_non_dma(void);
extern void spi_xfer_dma(u8* buf, u16 count, u8 is_blocking);

extern void debug_print(u8* text);

#endif
