#ifndef _MEW_FLASH_DRIVER_
#define _MEW_FLASH_DRIVER_

#include "../config.h"

#define MEW_FLASH_SPI                           SPI1

#define MEW_FLASH_SPI_GPIO_PINS			GPIO5 | GPIO6 | GPIO7
#define MEW_FLASH_SPI_GPIO_PORT			GPIOA
#define MEW_FLASH_SPI_GPIO_AF_NUMBER		GPIO_AF5

#define MEW_FLASH_GPIO_PORT_WP                  GPIOB
#define MEW_FLASH_GPIO_PIN_WP                   GPIO8

#define MEW_FLASH_GPIO_PORT_HOLD                GPIOC
#define MEW_FLASH_GPIO_PIN_HOLD                 GPIO0

#define MEW_FLASH_GPIO_PORT_CS                  GPIOC
#define MEW_FLASH_GPIO_PIN_CS                   GPIO1

#define MEW_FLASH_SECTOR_SIZE                   2048

void mew_spi_flash_init(void);
void mew_spi_flash_xfer(uint8_t* buffer_read, uint16_t count_read, uint8_t* buffer_write, uint16_t count_write);

void mew_flash_get_id(uint8_t* buf);

#endif