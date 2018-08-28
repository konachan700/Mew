#include "flash.h" 

/**************************************************************************************
 *  NOTE =^-^=
 *  I'm don't known, what is it - my mistake or HW bug...
 *  I can read SPI flash only on non-dma bidirectional mode. 
 *  It's already read as zero in unidirectional mode or bidirectional DMA mode.
 *  May be, it's QSPI, activated by default on flash? 
 **************************************************************************************/

void mew_spi_flash_init(void) {
    gpio_mode_setup(MEW_FLASH_GPIO_PORT_WP, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MEW_FLASH_GPIO_PIN_WP);
    gpio_set_output_options(MEW_FLASH_GPIO_PORT_WP, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, MEW_FLASH_GPIO_PIN_WP);
    gpio_set(MEW_FLASH_GPIO_PORT_WP, MEW_FLASH_GPIO_PIN_WP);
    
    gpio_mode_setup(MEW_FLASH_GPIO_PORT_HOLD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MEW_FLASH_GPIO_PIN_HOLD);
    gpio_set_output_options(MEW_FLASH_GPIO_PORT_HOLD, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, MEW_FLASH_GPIO_PIN_HOLD);
    gpio_set(MEW_FLASH_GPIO_PORT_HOLD, MEW_FLASH_GPIO_PIN_HOLD);
    
    gpio_mode_setup(MEW_FLASH_GPIO_PORT_CS, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MEW_FLASH_GPIO_PIN_CS);
    gpio_set_output_options(MEW_FLASH_GPIO_PORT_CS, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, MEW_FLASH_GPIO_PIN_CS);
    gpio_set(MEW_FLASH_GPIO_PORT_CS, MEW_FLASH_GPIO_PIN_CS);
    
    gpio_mode_setup(MEW_FLASH_SPI_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, MEW_FLASH_SPI_GPIO_PINS);
    gpio_set_output_options(MEW_FLASH_SPI_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, MEW_FLASH_SPI_GPIO_PINS);
    gpio_set_af(MEW_FLASH_SPI_GPIO_PORT, MEW_FLASH_SPI_GPIO_AF_NUMBER, MEW_FLASH_SPI_GPIO_PINS);
    
    spi_disable(MEW_FLASH_SPI);
    spi_set_master_mode(MEW_FLASH_SPI);
    spi_set_baudrate_prescaler(MEW_FLASH_SPI, SPI_CR1_BR_FPCLK_DIV_2);
    spi_set_clock_polarity_0(MEW_FLASH_SPI);
    spi_set_clock_phase_0(MEW_FLASH_SPI);
    spi_set_unidirectional_mode(MEW_FLASH_SPI);
    spi_enable_software_slave_management(MEW_FLASH_SPI);
    spi_send_msb_first(MEW_FLASH_SPI);
    spi_set_nss_high(MEW_FLASH_SPI);
    SPI_I2SCFGR(MEW_FLASH_SPI) &= ~SPI_I2SCFGR_I2SMOD;
    spi_disable_tx_buffer_empty_interrupt(MEW_FLASH_SPI);
    spi_disable_rx_buffer_not_empty_interrupt(MEW_FLASH_SPI);
    spi_disable_error_interrupt(MEW_FLASH_SPI);
    spi_disable_tx_dma(MEW_FLASH_SPI);
    spi_disable_rx_dma(MEW_FLASH_SPI);
    spi_set_dff_8bit(MEW_FLASH_SPI);
    spi_send_msb_first(MEW_FLASH_SPI);
    spi_enable(MEW_FLASH_SPI);
}

void mew_spi_flash_xfer(uint8_t* buffer_read, uint16_t count_read, uint8_t* buffer_write, uint16_t count_write) {
    int i;
    for (i=0; i<count_write; i++) {
        spi_xfer(MEW_FLASH_SPI, (uint8_t) buffer_write[i]);
    }
    for (i=0; i<count_read; i++) {
        buffer_read[i] = (uint8_t) spi_xfer(MEW_FLASH_SPI, 0x00);
    }
}
