#ifndef _CONFIG_MODE_MEW_
#define _CONFIG_MODE_MEW_

#include <libopencm3/stm32/gpio.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "board.h"
#include "mew_usb_cdc.h"
#include "crypt.h"

#define MEW_CM_INPUT_BUFFER_SIZE        1024
#define MEW_CM_OUT_MAX_PACKET_SIZE      48

#define MEW_CM_ERR_TOO_MANY_DATA        1
#define MEW_CM_OK                       0xFFFF

#define MEW_CM_PING                     0xAA
#define MEW_CM_IDENT                    0x01
#define MEW_CM_READ_PASSWD              0x02
#define MEW_CM_WRITE_PASSWD             0x03
#define MEW_CM_CONFIG_READ              0x04
#define MEW_CM_CONFIG_WRITE             0x05

#define MEW_CM_RET_OK                   0x00
#define MEW_CM_RET_ERROR                0x01
#define MEW_CM_RET_PING                 0x02
#define MEW_CM_RET_CRC_FAIL             0x03
#define MEW_CM_RET_DATA_ACCESS_FAIL     0x04

#define MEW_CRC_RET_OK                  1
#define MEW_CRC_RET_FAIL                0

#define MEW_CM_CLEANER_TIMER            5


struct mew_cm_command_header {
    u32 command;
    u32 data_crc32;
    u32 data_len;
} __attribute__((aligned(4),packed));

void __cm_buf_reset(void);


extern u32 mew_cm_push_raw(u8* raw, u8 len);
extern void mew_cm_poll(void);

extern u32 check_crc(u32 crc32, u32* data, u16 len);
extern void cm_timer_proc(void);

#endif
