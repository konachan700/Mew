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
#include <libopencm3/stm32/sdio.h>

typedef unsigned char u8;
typedef unsigned int  u16;
typedef unsigned long u32;

#define LIGHT_UI_THEME

#define SPI_RX_DUMMY_BUF_SIZE 16
#define I2C_TIMEOUT (1024 * 1024)

#define SDIO_GENERAL_ERROR      1
#define SDIO_OK                 2

#define SDIO_MAX_CMD_RETRIES    5

#define SDIO_ESUCCESS           1
#define SDIO_EINPROGRESS        2
#define SDIO_ECTIMEOUT          3
#define SDIO_ECCRCFAIL          4
#define SDIO_EUNKNOWN           5

#define SDIO_CMD41_TIMER        0x290 // it's magic =)

#define SDIO_OCR_BUSY           0x80000000
#define SDIO_OCR_HCS            0x40000000
#define SDIO_OCR_CCS            0x40000000

#define SDIO_CMD_GO_IDLE_STATE          0

#define SDIO_ACMD_SEND_OP_COND          41

#define SDIO_CSD_V1_READ_BLOCK_LEN      ((SDIO_RESP2 >> 16) & 0xF)
#define SDIO_CSD_V1_BLOCK_LEN           (1 << SDIO_CSD_V1_READ_BLOCK_LEN)
#define SDIO_CSD_V1_C_SIZE              (((SDIO_RESP2 & 0x3FF) << 2) | (SDIO_RESP3 >> 30))
#define SDIO_CSD_V1_C_SIZE_MULT         ((SDIO_RESP3 >> 15) & 0x7)
#define SDIO_CSD_V1_MULT                (1 << (SDIO_CSD_V1_C_SIZE_MULT + 2))
#define SDIO_CSD_V1_BLOCK_NR            ((SDIO_CSD_V1_C_SIZE + 1) * SDIO_CSD_V1_MULT)

#define SDIO_CSD_V2_C_SIZE              (((SDIO_RESP2 & 0x3F) << 16) | (SDIO_RESP3 >> 16))

#define DATA_RX_ERROR_FLAGS             (SDIO_STA_STBITERR | SDIO_STA_RXOVERR | SDIO_STA_DTIMEOUT | SDIO_STA_DCRCFAIL)
#define DATA_RX_SUCCESS_FLAGS           (SDIO_STA_DBCKEND | SDIO_STA_DATAEND)

struct dma1_i2c1_transaction {
    u8  dev_addr;
    u8  read_write;
    u16 buffer_count;
    u8* buffer;
    u32 last_error;
    u32 i2c_sr1;
    u32 i2c_sr2;
};

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ui.h"
#include "ILI9341.h"
#include "menu.h"

extern void start_debug_usart(void);
extern void start_all_clock(void);
extern void start_timer_2(void);
extern void start_backlight(void);
extern void start_buttons(void);
extern void start_leds(void);

extern void start_i2c1(void);
extern void i2c_dma_req(struct dma1_i2c1_transaction* i2c_tr);
extern void i2c_read_dma_wait(void);
extern void i2c_write_dma_wait(void);

extern u32 i2c_fram_read_dma(u8 page, u8 start_byte, u8* buffer, u16 count);
extern u32 i2c_fram_write_dma(u8 page, u8 start_byte, u8* buffer, u16 count);

extern void start_spi_2_dma(void);
extern void start_spi_2_non_dma(void);
extern void spi_xfer_dma(u8* buf, u16 count, u8 is_blocking);

extern void debug_print(u8* text);
extern void debug_print_hex(u8* blob, u16 len);

extern void start_sdio(void);
extern u32 sdio_read_b512(u32 address, u32 *buffer);

#endif
