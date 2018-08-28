#include "../config.h"
#include"flash.h"

static const uint8_t MEW_SPI_FLASH_CMD__READ_ID[] = { 0x9f, 0x00 };

uint8_t ZRAM _mew_flash_buffer[MEW_FLASH_SECTOR_SIZE];

void mew_flash_get_id(uint8_t* buf) {
    gpio_clear(MEW_FLASH_GPIO_PORT_CS, MEW_FLASH_GPIO_PIN_CS);
    mew_spi_flash_xfer(buf, 3, (uint8_t *) MEW_SPI_FLASH_CMD__READ_ID, 2);
    gpio_set(MEW_FLASH_GPIO_PORT_CS, MEW_FLASH_GPIO_PIN_CS);
}
