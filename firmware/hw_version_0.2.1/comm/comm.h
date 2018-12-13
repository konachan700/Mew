#ifndef COMM_H
#define COMM_H

#include "../config.h"
#include "../crypto/crypto.h"

#define MEW_COMM_MAGIC_1 0x43
#define MEW_COMM_MAGIC_2 0x77

#define MEW_COMM_TIMEOUT_MS             30
#define MEW_COMM_HEADER_MAX_SIZE        16      // 2 byte magic + 2 byte length + 4 byte crc32 + 8 bytes reserved
#define MEW_COMM_PAYLOAD_MAX_SIZE       1024
#define MEW_COMM_BUF_MAX_SIZE           (MEW_COMM_PAYLOAD_MAX_SIZE + MEW_COMM_HEADER_MAX_SIZE)  

#define MEW_COMM_BYTES_TO_UINT32(b1,b2,b3,b4)    ((b1 << 24) + (b2 << 16) + (b3 << 8) + b4)
#define MEW_COMM_BYTES_TO_UINT16(b1,b2)          ((b1 << 8) + b2)

#define MEW_COMM_ERR_DRIVER_COLLISION           0xE0
#define MEW_COMM_ERR_BAD_PACKAGE_MAGIC          0xE1
#define MEW_COMM_ERR_BAD_PAYLOAD_SIZE           0xE2
#define MEW_COMM_ERR_BAD_PAYLOAD_CRC32          0xE3
#define MEW_COMM_ERR_BUFFER_OVERFLOW            0xE4
#define MEW_COMM_ERR_BISY                       0xE5
#define MEW_COMM_ERR_TIMEOUT                    0xE6

uint32_t mew_comm_is_payload_present(void);

void mew_comm_handler(void);
void mew_comm_add_byte(uint32_t driver_id, uint8_t b);
void mew_comm_get_payload(uint8_t** payload, uint32_t* length);

#endif /* COMM_H */

