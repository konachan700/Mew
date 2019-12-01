#include "flash.h"
#include "debug.h"

static uint8_t _mew_flash_read_buffer[MEW_FLASH_SECTOR_SIZE];
//static uint8_t _mew_flash_write_buffer[MEW_FLASH_SECTOR_SIZE];

static uint8_t _mew_cmd_read_id[] = { 0x9f, 0x00 };
static uint8_t _mew_cmd_fast_read[] = { 0x0b, 0x00, 0x00, 0x00 };

void mew_flash_get_id(uint8_t* buf) {
    mew_spi_flash_xfer(buf, _mew_cmd_read_id, 3, 2, MEW_FLASH_SYNC_OP);
}

static void __mew_flash_read_sector_async_handler(void) {

}

void mew_flash_read_sector_async(uint16_t addr, void (*mew_flash_rx_handler)(uint8_t * buf)) {
	mew_spi_flash_set_on_receive_handler(&__mew_flash_read_sector_async_handler);
	_mew_cmd_fast_read[1] = (addr << 8) & 0xFF;
	_mew_cmd_fast_read[2] = addr & 0xFF;
	mew_spi_flash_xfer(_mew_flash_read_buffer, _mew_cmd_fast_read, 2048, 4, MEW_FLASH_ASYNC_OP);
}
