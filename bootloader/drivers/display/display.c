#include "mew.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

#include "display.h"
#include "debug.h"
#include "../system/system.h"

extern const uint8_t mew_logo_width;
extern const uint8_t mew_logo_height;
extern const uint8_t mew_logo_pixel_map[];

extern const unsigned int mew_sync_logo_width;
extern const unsigned int mew_sync_logo_height;
extern const unsigned char *mew_sync_logo_pixel_map;

static void __mew_wait(void);
static void __mew_cmd(void);
static void __mew_data(void);
static void __mew_unselect(void);

static void __mew_init_control_pins(void);
static void __mew_init_spi_pins(void);
static void __mew_spi_init_non_dma(void);
static void __mew_display_init(void);

static void __mew_display_write_cmd(uint16_t cmd);
static void __mew_display_write_cmd_with_data(uint16_t cmd, uint16_t* data, uint16_t size);

static void __mew_spi_fill_dma(const char * dummy, uint16_t count);
static void mew_display_flush(int x1, int y1, int x2, int y2, const unsigned char* color_p);
static void mew_display_flush_clear(void);

static volatile uint8_t _mew_dma_rx_state = 0;

static void __mew_wait(void) {
    uint16_t i;
    for (i=0; i<128; i++) __asm__("NOP");
}

static void __mew_data(void) {
    gpio_clear(MEW_DISPLAY_CS_PORT, MEW_DISPLAY_CS_PIN);
    gpio_set(MEW_DISPLAY_DC_PORT, MEW_DISPLAY_DC_PIN);
    __mew_wait();
}

static void __mew_cmd(void) {
    gpio_clear(MEW_DISPLAY_CS_PORT, MEW_DISPLAY_CS_PIN);
    gpio_clear(MEW_DISPLAY_DC_PORT, MEW_DISPLAY_DC_PIN);
    __mew_wait();
}

static void __mew_unselect(void) {
    gpio_set(MEW_DISPLAY_CS_PORT, MEW_DISPLAY_CS_PIN);
    gpio_set(MEW_DISPLAY_DC_PORT, MEW_DISPLAY_DC_PIN);
    __mew_wait();
}

static void __mew_display_write_cmd(uint16_t cmd) {
    __mew_cmd();
    spi_send(MEW_DISPLAY_SPI, cmd);
    __mew_wait();
}

static void __mew_display_write_cmd_with_data(uint16_t cmd, uint16_t* data, uint16_t size) {
    uint16_t i;
    __mew_cmd();
    spi_send(MEW_DISPLAY_SPI, cmd);
    __mew_wait();
    __mew_data();
    for (i=0; i<size; i++) {
        spi_send(MEW_DISPLAY_SPI, data[i]);
    }
    __mew_wait();
}

static void __mew_display_init(void) {
    gpio_clear(MEW_DISPLAY_RESET_PORT, MEW_DISPLAY_RESET_PIN);
    mew_delay_ms(10);
    gpio_set(MEW_DISPLAY_RESET_PORT, MEW_DISPLAY_RESET_PIN);
    mew_delay_ms(50);
    
    __mew_display_write_cmd(0x01);
    mew_delay_ms(50);
    __mew_display_write_cmd(0x28);
    
    __mew_display_write_cmd_with_data(0xCB, (uint16_t[]) { 0x39,0x2C,0x00,0x34,0x02 }, 5);
    __mew_display_write_cmd_with_data(0xCF, (uint16_t[]) { 0x00,0xC1,0x30 }, 3);
    __mew_display_write_cmd_with_data(0xE8, (uint16_t[]) { 0x85,0x00,0x78 }, 3);
    __mew_display_write_cmd_with_data(0xEA, (uint16_t[]) { 0x00,0x00 }, 2);
    __mew_display_write_cmd_with_data(0xED, (uint16_t[]) { 0x64,0x03,0x12,0x81 }, 4);
    __mew_display_write_cmd_with_data(0xF7, (uint16_t[]) { 0x20 }, 1);
    __mew_display_write_cmd_with_data(0xC0, (uint16_t[]) { 0x23 }, 1);
    __mew_display_write_cmd_with_data(0xC1, (uint16_t[]) { 0x10 }, 1);
    __mew_display_write_cmd_with_data(0xC5, (uint16_t[]) { 0x3e,0x28 }, 2);
    __mew_display_write_cmd_with_data(0xC7, (uint16_t[]) { 0x86 }, 1);
    __mew_display_write_cmd_with_data(0x36, (uint16_t[]) { 0x9C }, 1); // bit 4 - RGB/BGR  0101 1010
    __mew_display_write_cmd_with_data(0x3A, (uint16_t[]) { 0x55 }, 1);
    __mew_display_write_cmd_with_data(0xB1, (uint16_t[]) { 0x00,0x18 }, 2);
    __mew_display_write_cmd_with_data(0xB6, (uint16_t[]) { 0x08,0x82,0x27 }, 3);
    __mew_display_write_cmd_with_data(0xF2, (uint16_t[]) { 0x00 }, 1);
    __mew_display_write_cmd_with_data(0x26, (uint16_t[]) { 0x01 }, 1);
    __mew_display_write_cmd_with_data(0xE0, (uint16_t[]) { 0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00 }, 15);
    __mew_display_write_cmd_with_data(0xE1, (uint16_t[]) { 0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F }, 15);
    __mew_display_write_cmd_with_data(0x33, (uint16_t[]) { 0x00,0x00,0x01,0x40,0x00,0x00 }, 6);
    __mew_display_write_cmd_with_data(0x37, (uint16_t[]) { 0x00,0x00 }, 2);
    __mew_display_write_cmd_with_data(0x53, (uint16_t[]) { 0x20 }, 1);
    __mew_display_write_cmd_with_data(0x51, (uint16_t[]) { 0xFF }, 1);

    __mew_display_write_cmd(0x11);
    mew_delay_ms(100);
    __mew_display_write_cmd(0x29);
    mew_delay_ms(20);
}

static void __mew_init_control_pins(void) {
    gpio_mode_setup(MEW_DISPLAY_RESET_PORT, GPIO_MODE_OUTPUT, GPIO_MODE_OUTPUT, MEW_DISPLAY_RESET_PIN);
    gpio_set_output_options(MEW_DISPLAY_RESET_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, MEW_DISPLAY_RESET_PIN);
    
    gpio_mode_setup(MEW_DISPLAY_DC_PORT, GPIO_MODE_OUTPUT, GPIO_MODE_OUTPUT, MEW_DISPLAY_DC_PIN);
    gpio_set_output_options(MEW_DISPLAY_DC_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, MEW_DISPLAY_DC_PIN);
    
    gpio_mode_setup(MEW_DISPLAY_CS_PORT, GPIO_MODE_OUTPUT, GPIO_MODE_OUTPUT, MEW_DISPLAY_CS_PIN);
    gpio_set_output_options(MEW_DISPLAY_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, MEW_DISPLAY_CS_PIN);
    
    gpio_mode_setup(MEW_DISPLAY_BL_PORT, GPIO_MODE_OUTPUT, GPIO_MODE_OUTPUT, MEW_DISPLAY_BL_PIN);
    gpio_set_output_options(MEW_DISPLAY_BL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, MEW_DISPLAY_BL_PIN);
    gpio_set(MEW_DISPLAY_BL_PORT, MEW_DISPLAY_BL_PIN);
}

static void __mew_init_spi_pins(void) {
    gpio_mode_setup(MEW_DISPLAY_SPI_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MEW_DISPLAY_SPI_GPIO_PINS);
    gpio_set_output_options(MEW_DISPLAY_SPI_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, MEW_DISPLAY_SPI_GPIO_PINS);
    gpio_set_af(MEW_DISPLAY_SPI_GPIO_PORT, MEW_DISPLAY_SPI_GPIO_AF_NUMBER, MEW_DISPLAY_SPI_GPIO_PINS);
}

static void __mew_spi_init_non_dma(void) {
    nvic_disable_irq(MEW_DISPLAY_DMA_NVIC);
    spi_disable(MEW_DISPLAY_SPI);
    spi_set_master_mode(MEW_DISPLAY_SPI);
    spi_set_baudrate_prescaler(MEW_DISPLAY_SPI, SPI_CR1_BR_FPCLK_DIV_4);
    spi_set_clock_polarity_0(MEW_DISPLAY_SPI);
    spi_set_clock_phase_0(MEW_DISPLAY_SPI);
    spi_set_unidirectional_mode(MEW_DISPLAY_SPI);
    spi_enable_software_slave_management(MEW_DISPLAY_SPI);
    spi_send_msb_first(MEW_DISPLAY_SPI);
    spi_set_nss_high(MEW_DISPLAY_SPI);
    spi_disable_tx_buffer_empty_interrupt(MEW_DISPLAY_SPI);
    spi_disable_rx_buffer_not_empty_interrupt(MEW_DISPLAY_SPI);
    spi_disable_error_interrupt(MEW_DISPLAY_SPI);
    spi_disable_tx_dma(MEW_DISPLAY_SPI);
    spi_disable_rx_dma(MEW_DISPLAY_SPI);
    spi_set_dff_8bit(MEW_DISPLAY_SPI);
    spi_send_msb_first(MEW_DISPLAY_SPI);
    spi_enable(MEW_DISPLAY_SPI);
}

static void __mew_spi_init_dma(void) {
    spi_disable(MEW_DISPLAY_SPI);
    spi_reset(MEW_DISPLAY_SPI);
    spi_set_master_mode(MEW_DISPLAY_SPI);
    spi_set_baudrate_prescaler(MEW_DISPLAY_SPI, SPI_CR1_BR_FPCLK_DIV_2);
    spi_set_clock_polarity_0(MEW_DISPLAY_SPI);
    spi_set_clock_phase_0(MEW_DISPLAY_SPI);
    spi_set_bidirectional_transmit_only_mode(MEW_DISPLAY_SPI);
    spi_enable_software_slave_management(MEW_DISPLAY_SPI);
    spi_set_nss_high(MEW_DISPLAY_SPI);
    spi_disable_tx_buffer_empty_interrupt(MEW_DISPLAY_SPI);
    spi_disable_rx_buffer_not_empty_interrupt(MEW_DISPLAY_SPI);
    spi_disable_error_interrupt(MEW_DISPLAY_SPI);
    spi_set_dff_16bit(MEW_DISPLAY_SPI);
    spi_send_msb_first(MEW_DISPLAY_SPI);
    spi_enable_tx_dma(MEW_DISPLAY_SPI);
    spi_enable(MEW_DISPLAY_SPI);
    nvic_enable_irq(MEW_DISPLAY_DMA_NVIC);
}

static void __mew_spi_write_dma(const short* buffer, uint16_t count) {
    mew_check_dma_memory((void*)buffer, "__mew_spi_write_dma_i");
    dma_set_transfer_mode(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(MEW_DISPLAY_DMA,  MEW_DISPLAY_DMA_STREAM, DMA_SxCR_PL_VERY_HIGH);
    dma_set_memory_size(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, DMA_SxCR_MSIZE_16BIT);
    dma_set_peripheral_size(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, DMA_SxCR_PSIZE_16BIT);
    dma_enable_memory_increment_mode(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_set_peripheral_address(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, (uint32_t) &MEW_DISPLAY_DMA_DR);
    dma_channel_select(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, MEW_DISPLAY_DMA_CHANNEL);
    dma_set_memory_address(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, (uint32_t) buffer);
    dma_set_number_of_data(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, count);
    dma_disable_fifo_error_interrupt(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_disable_half_transfer_interrupt(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_enable_transfer_complete_interrupt(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_enable_transfer_error_interrupt(MEW_DISPLAY_DMA,    MEW_DISPLAY_DMA_STREAM);
    dma_enable_transfer_complete_interrupt(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_enable_stream(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
}

static void __mew_spi_fill_dma(const char * dummy, uint16_t count) {
	_mew_dma_rx_state = 1;

    dma_set_transfer_mode(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(MEW_DISPLAY_DMA,  MEW_DISPLAY_DMA_STREAM, DMA_SxCR_PL_VERY_HIGH);
    dma_set_memory_size(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, DMA_SxCR_MSIZE_16BIT);
    dma_set_peripheral_size(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, DMA_SxCR_PSIZE_16BIT);
    dma_disable_memory_increment_mode(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_set_peripheral_address(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, (uint32_t) &MEW_DISPLAY_DMA_DR);
    dma_channel_select(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, MEW_DISPLAY_DMA_CHANNEL);
    dma_set_memory_address(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, (uint32_t) dummy);
    dma_set_number_of_data(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM, count);
    dma_disable_fifo_error_interrupt(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_disable_half_transfer_interrupt(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_enable_transfer_complete_interrupt(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_enable_transfer_error_interrupt(MEW_DISPLAY_DMA,    MEW_DISPLAY_DMA_STREAM);
    dma_enable_transfer_complete_interrupt(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_enable_stream(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);

    while (_mew_dma_rx_state == 1) __asm__("NOP");
}

unsigned int mew_display_init(void) {
	const int x1 = (240 / 2) - (mew_logo_width / 2);
	const int y1 = (320 / 2) - (mew_logo_height / 2);

    __mew_init_control_pins();
    __mew_init_spi_pins();
    __mew_spi_init_non_dma();
    __mew_display_init();

    mew_display_flush_clear();

    mew_display_flush(x1, y1, x1 + mew_logo_width - 1, y1 + mew_logo_height - 1, mew_logo_pixel_map);

    return 0;
}

void mew_display_sync_icon(void) {
	const int x1 = (240 / 2) - (mew_sync_logo_width / 2);
	const int y1 = (320 - mew_sync_logo_height - 32);

	mew_display_flush(x1, y1, x1 + mew_sync_logo_width - 1,
			y1 + mew_sync_logo_height - 1, mew_sync_logo_pixel_map);
}

static void mew_display_flush_clear(void) {
	const char dummy[] = {0xFF, 0xFF};

    __mew_spi_init_non_dma();
    __mew_display_write_cmd_with_data(0x2A, (uint16_t[]) { 0, 0, 0, 239 }, 4);
    __mew_display_write_cmd_with_data(0x2B, (uint16_t[]) { 0, 0, 1, 319 & 0xFF }, 4);
    __mew_display_write_cmd(0x2c);
    __mew_data();

    _mew_dma_rx_state = 1;
    __mew_spi_init_dma();

    __mew_spi_fill_dma(dummy, 60000);
    __mew_spi_fill_dma(dummy, 60000);
    __mew_spi_fill_dma(dummy, 33600);
}

static void mew_display_flush(int x1, int y1, int x2, int y2, const unsigned char* color_p) {
    while (_mew_dma_rx_state == 1) __asm__("NOP");

    __mew_spi_init_non_dma();
    __mew_display_write_cmd_with_data(0x2A, (uint16_t[]) { (x1 >> 8), x1, (x2 >> 8), x2 }, 4);
    __mew_display_write_cmd_with_data(0x2B, (uint16_t[]) { (y1 >> 8), y1, (y2 >> 8), y2 }, 4);
    __mew_display_write_cmd(0x2c);
    __mew_data();
    
    _mew_dma_rx_state = 1;
    __mew_spi_init_dma();
    __mew_spi_write_dma((const short *) color_p, ((y2 - y1 + 1) * (x2 - x1 + 1)));

    while (_mew_dma_rx_state == 1) __asm__("NOP");
}

void MEW_DISPLAY_DMA_HANDLE(void) {
    dma_stream_reset(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_disable_stream(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    _mew_dma_rx_state = 0;
    //__mew_unselect();
}
