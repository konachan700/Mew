#ifndef COMM_H
#define COMM_H

#include "mew.h"
#include "../../drivers/system/system.h"
#include "../../drivers/bluetooth/bluetooth.h"

#include <libopencm3/stm32/crc.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEW_COMM_MAGIC_1                        0x43
#define MEW_COMM_MAGIC_2                        0x77

#define MEW_COMM_TIMEOUT_MS                     150
#define MEW_COMM_HEADER_MAX_SIZE                10      // 2 byte magic + 2 bytes cmd + 2 byte length + 4 byte crc32
#define MEW_COMM_PAYLOAD_MAX_SIZE               2048
#define MEW_COMM_BUF_MAX_SIZE                   (MEW_COMM_PAYLOAD_MAX_SIZE + MEW_COMM_HEADER_MAX_SIZE)  

#if MEW_COMM_PAYLOAD_MAX_SIZE % 4 != 0
#error "Incorrect payload size"
#endif

//#define MEW_COMM_BYTES_TO_UINT32(b1,b2,b3,b4)   ((b1 << 24) + (b2 << 16) + (b3 << 8) + b4)
//#define MEW_COMM_BYTES_TO_UINT16(b1,b2)         ((b1 << 8) + b2)

#define MEW_COMM_IS_MAGIC_INVALID(pb)			((pb[0] != MEW_COMM_MAGIC_1) || (pb[1] != MEW_COMM_MAGIC_2))
#define MEW_COMM_GET_COMMAND(pb)				((pb[2] << 8) + pb[3])
#define MEW_COMM_GET_PL_SIZE(pb)				((pb[4] << 8) + pb[5])
#define MEW_COMM_GET_PL_CRC32(pb)				((pb[6] << 24) + (pb[7] << 16) + (pb[8] << 8) + pb[9])

#define MEW_SHL0(i)								(uint8_t)((i >>  0) & 0xFF)
#define MEW_SHL8(i)								(uint8_t)((i >>  8) & 0xFF)
#define MEW_SHL16(i)							(uint8_t)((i >> 16) & 0xFF)
#define MEW_SHL24(i)							(uint8_t)((i >> 24) & 0xFF)

#define MEW_COMM_ERR_DRIVER_COLLISION           0xE0
#define MEW_COMM_ERR_BAD_PACKAGE_MAGIC          0xE1
#define MEW_COMM_ERR_BAD_PAYLOAD_SIZE           0xE2
#define MEW_COMM_ERR_BAD_PAYLOAD_CRC32          0xE3
#define MEW_COMM_ERR_BUFFER_OVERFLOW            0xE4
#define MEW_COMM_ERR_BISY                       0xE5
#define MEW_COMM_ERR_TIMEOUT                    0xE6

#define MEW_COMM_GET_DEVICE_SESSION_KEY			0x4300
#define MEW_COMM_SEND_HOST_SESSION_KEY   		0x4301

uint32_t mew_comm_is_payload_present(void);

unsigned int mew_comm_handler(void);
void mew_comm_add_byte(uint32_t driver_id, uint8_t b);
void mew_comm_get_payload(uint8_t* payload, uint32_t* length);

#endif /* COMM_H */

