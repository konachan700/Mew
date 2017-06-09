#ifndef __BOARD_MEW_SDIO__
#define __BOARD_MEW_SDIO__

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

#include "board.h" 

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SDIO_R1_IDLE   0
#define SDIO_R1_READY  1
#define SDIO_R1_IDENT  2
#define SDIO_R1_STDBY  3
#define SDIO_R1_TRAN   4
#define SDIO_R1_DATA   5
#define SDIO_R1_RCV    6
#define SDIO_R1_PRG    7
#define SDIO_R1_DIS    8
#define SDIO_R1_ERROR  255

#define SDIO_GENERAL_ERROR      1
#define SDIO_OK                 2

#define SDIO_WRITE              1
#define SDIO_READ               2

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

#define DATA_TX_ERROR_FLAGS             (SDIO_STA_STBITERR | SDIO_STA_TXUNDERR | SDIO_STA_DTIMEOUT | SDIO_STA_DCRCFAIL)
#define DATA_TX_SUCCESS_FLAGS           (SDIO_STA_DBCKEND | SDIO_STA_DATAEND) 

extern void start_sdio(void);
extern u32 sdio_rw512(u8 rw, u32 address, u32 *buffer);

#endif
