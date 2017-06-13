#ifndef __BOARD_MEW_BSP__
#define __BOARD_MEW_BSP__
//#define STM32F2

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
#include <libopencm3/stm32/sdio.h>
#include <libopencm3/stm32/crc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/scb.h> 

#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/cm3/common.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef signed int s16;
typedef signed long s32;

#define MEW_GLOBAL_MODE_HID 0x00000000UL
#define MEW_GLOBAL_MODE_CDC 0xFFFFFFFFUL
#define MEW_GLOBAL_MODE_MSD 0x11111111UL

#define LIGHT_UI_THEME

#define CREATE_DEMO_PASSWORDS

#define SPI_RX_DUMMY_BUF_SIZE 16

#define MEW_SETTINGS_EEPROM_PAGE_OFFSET 0

struct dma1_i2c1_transaction {
    u8  dev_addr;
    u8  read_write;
    u16 main_buffer_count;
    u8* main_buffer;  
    u32 last_error;
    u32 i2c_sr1;
    u32 i2c_sr2;
    u32 flags;
};

#include "font_icons_gmd.h"

extern void start_random(void);
extern u32 random_u32(void);
extern void memset_random_u32(u32* data, u16 len);
extern u32 crc_gen(u32* data, u16 len);

extern void start_debug_usart(void);
extern void start_all_clock(void);
extern void start_timer_2(void);
extern void start_backlight(void);
extern void start_buttons(void);
extern void start_leds(void);
extern void start_i2c1(void);

extern u32 i2c_fram_read_dma(u8 page, u8 start_byte, u8* buffer, u16 count);
extern u32 i2c_fram_write_dma(u8 page, u8 start_byte, u8* buffer, u16 count);

extern void start_spi_2_dma(void);
extern void start_spi_2_non_dma(void);
extern void spi_xfer_dma(u8* buf, u16 count, u8 is_blocking);

extern void debug_print(u8* text);
extern void debug_print_hex(u8* blob, u16 len);

#endif
