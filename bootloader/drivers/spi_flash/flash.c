#include "flash.h" 
#include "../system/system.h"
#include "debug.h"

static void __mew_spi_dma_send(void);
static void __mew_spi_dma_receive(void);
static void __mew_spi_dma_reset_rx_buffer(void);

static void __mew_spi_get_flash_id(void);

#define MEW_FLASH_XFER_STATE_SENDING 	0x03
#define MEW_FLASH_XFER_STATE_RECEIVING	0x07
#define MEW_FLASH_XFER_STATE_FREE 		0x00

static volatile uint32_t mew_flash_xfer_state = MEW_FLASH_XFER_STATE_FREE;

static volatile uint32_t mew_flash_mem_addr_sending = 0;
static volatile uint16_t mew_flash_mem_size_sending = 0;

static volatile uint32_t mew_flash_mem_addr_receiving = 0;
static volatile uint16_t mew_flash_mem_size_receiving = 0;

void (*mew_flash_rx_handler)(void) = NULL;

static uint8_t mew_flash_dma_dummy[1] = { '0' };

unsigned int mew_spi_flash_init(void) {
    gpio_mode_setup(MEW_FLASH_GPIO_PORT_WP, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, MEW_FLASH_GPIO_PIN_WP);
    gpio_set_output_options(MEW_FLASH_GPIO_PORT_WP, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, MEW_FLASH_GPIO_PIN_WP);
    gpio_set(MEW_FLASH_GPIO_PORT_WP, MEW_FLASH_GPIO_PIN_WP);
    
    gpio_mode_setup(MEW_FLASH_GPIO_PORT_HOLD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, MEW_FLASH_GPIO_PIN_HOLD);
    gpio_set_output_options(MEW_FLASH_GPIO_PORT_HOLD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, MEW_FLASH_GPIO_PIN_HOLD);
    gpio_set(MEW_FLASH_GPIO_PORT_HOLD, MEW_FLASH_GPIO_PIN_HOLD);
    
    gpio_mode_setup(MEW_FLASH_GPIO_PORT_CS, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MEW_FLASH_GPIO_PIN_CS);
    gpio_set_output_options(MEW_FLASH_GPIO_PORT_CS, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, MEW_FLASH_GPIO_PIN_CS);
    gpio_set(MEW_FLASH_GPIO_PORT_CS, MEW_FLASH_GPIO_PIN_CS);
    
    gpio_mode_setup(MEW_FLASH_SPI_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, MEW_FLASH_SPI_GPIO_PINS);
    gpio_set_output_options(MEW_FLASH_SPI_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, MEW_FLASH_SPI_GPIO_PINS);
    gpio_set_af(MEW_FLASH_SPI_GPIO_PORT, MEW_FLASH_SPI_GPIO_AF_NUMBER, MEW_FLASH_SPI_GPIO_PINS);
    
    spi_disable(MEW_FLASH_SPI);
    spi_set_master_mode(MEW_FLASH_SPI);
    spi_set_baudrate_prescaler(MEW_FLASH_SPI, SPI_CR1_BR_FPCLK_DIV_2);
    spi_set_clock_polarity_0(MEW_FLASH_SPI);
    spi_set_clock_phase_0(MEW_FLASH_SPI);
    spi_send_msb_first(MEW_FLASH_SPI);
    spi_set_nss_high(MEW_FLASH_SPI);
    spi_enable_software_slave_management(MEW_FLASH_SPI);
    spi_set_dff_8bit(MEW_FLASH_SPI);
    spi_disable_ss_output(MEW_FLASH_SPI);
    spi_enable_rx_dma(MEW_FLASH_SPI);
    spi_enable_tx_dma(MEW_FLASH_SPI);
    spi_set_unidirectional_mode(MEW_FLASH_SPI);
    spi_set_full_duplex_mode(MEW_FLASH_SPI);
    spi_enable(MEW_FLASH_SPI);

    nvic_enable_irq(MEW_FLASH_NVIC_RX);
    nvic_enable_irq(MEW_FLASH_NVIC_TX);

    __mew_spi_get_flash_id();

    return 0;
}

static void __mew_spi_get_flash_id(void) {
    uint8_t buf[3];
    memset((void*)buf, 0, 3);
    mew_flash_get_id(buf);
    mew_duart_print_hex_ex("SPI flash ID: ", (const char*)buf, 3);
}

void MEW_FLASH_DMA_RX_HANDLE(void) {
    dma_stream_reset(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX);
    dma_disable_stream(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX);

    mew_flash_xfer_state = MEW_FLASH_XFER_STATE_FREE;
    gpio_set(MEW_FLASH_GPIO_PORT_CS, MEW_FLASH_GPIO_PIN_CS);
//    mew_debug_print("Spi read ok");
}

void MEW_FLASH_DMA_TX_HANDLE(void) {
    dma_stream_reset(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
    dma_disable_stream(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);

    if (mew_flash_xfer_state == MEW_FLASH_XFER_STATE_SENDING) {
    	__mew_spi_dma_reset_rx_buffer();
    	if (mew_flash_mem_size_receiving > 0) {
    		__mew_spi_dma_receive();
    	} else {
    		mew_flash_xfer_state = MEW_FLASH_XFER_STATE_FREE;
    		gpio_set(MEW_FLASH_GPIO_PORT_CS, MEW_FLASH_GPIO_PIN_CS);
    	}
    }
//    mew_debug_print("Spi send ok");
}

static void __mew_spi_dma_send(void) {
	mew_flash_xfer_state = MEW_FLASH_XFER_STATE_SENDING;

    dma_set_transfer_mode(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(MEW_FLASH_DMA,  MEW_FLASH_DMA_STREAM_TX, DMA_SxCR_PL_VERY_HIGH);
    dma_set_memory_size(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, DMA_SxCR_PSIZE_8BIT);
    dma_enable_memory_increment_mode(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
    dma_set_peripheral_address(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, (uint32_t) &MEW_FLASH_SPI_DR);
    dma_channel_select(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, MEW_SPI_DMA_CHANNEL);
    dma_set_memory_address(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, (uint32_t) mew_flash_mem_addr_sending);
    dma_set_number_of_data(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, mew_flash_mem_size_sending);
    dma_disable_fifo_error_interrupt(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
    dma_disable_half_transfer_interrupt(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
    dma_enable_transfer_complete_interrupt(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
    dma_disable_transfer_error_interrupt(MEW_FLASH_DMA,    MEW_FLASH_DMA_STREAM_TX);
    dma_enable_stream(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
}

static void __mew_spi_dma_receive(void) {
	mew_flash_xfer_state = MEW_FLASH_XFER_STATE_RECEIVING;

    dma_set_transfer_mode(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
    dma_set_priority(MEW_FLASH_DMA,  MEW_FLASH_DMA_STREAM_RX, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX, DMA_SxCR_PSIZE_8BIT);
    dma_enable_memory_increment_mode(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX);
    dma_set_peripheral_address(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX, (uint32_t) &MEW_FLASH_SPI_DR);
    dma_channel_select(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX, MEW_SPI_DMA_CHANNEL);
    dma_set_memory_address(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX, mew_flash_mem_addr_receiving);
    dma_set_number_of_data(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX, mew_flash_mem_size_receiving);
    dma_disable_fifo_error_interrupt(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX);
    dma_disable_half_transfer_interrupt(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX);
    dma_enable_transfer_complete_interrupt(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX);
    dma_disable_transfer_error_interrupt(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX);

    dma_set_transfer_mode(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(MEW_FLASH_DMA,  MEW_FLASH_DMA_STREAM_TX, DMA_SxCR_PL_VERY_HIGH);
    dma_set_memory_size(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, DMA_SxCR_PSIZE_8BIT);
    dma_disable_memory_increment_mode(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
    dma_set_peripheral_address(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, (uint32_t) &MEW_FLASH_SPI_DR);
    dma_channel_select(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, MEW_SPI_DMA_CHANNEL);
    dma_set_memory_address(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, (uint32_t) mew_flash_dma_dummy);
    dma_set_number_of_data(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX, mew_flash_mem_size_receiving);
    dma_disable_fifo_error_interrupt(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
    dma_disable_half_transfer_interrupt(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
    dma_enable_transfer_complete_interrupt(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
    dma_disable_transfer_error_interrupt(MEW_FLASH_DMA,    MEW_FLASH_DMA_STREAM_TX);

    dma_enable_stream(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_RX);
    dma_enable_stream(MEW_FLASH_DMA, MEW_FLASH_DMA_STREAM_TX);
}

static void __mew_spi_dma_reset_rx_buffer(void) {
    while (SPI_SR(MEW_FLASH_SPI) & SPI_SR_RXNE) {
    	mew_flash_dma_dummy[0] = SPI_DR(MEW_FLASH_SPI);
    }
}

void mew_spi_flash_xfer(uint8_t* buffer_read, uint8_t* buffer_write, uint16_t read_count, uint16_t write_count, uint8_t op_type) {
    mew_check_dma_memory((void*)buffer_write, "mew_spi_flash_xfer buffer_write");
    mew_check_dma_memory((void*)buffer_read, "mew_spi_flash_xfer buffer_read");

    while (mew_flash_xfer_state != MEW_FLASH_XFER_STATE_FREE) {};

    gpio_clear(MEW_FLASH_GPIO_PORT_CS, MEW_FLASH_GPIO_PIN_CS);

    mew_flash_mem_addr_sending = (uint32_t) buffer_write;
    mew_flash_mem_size_sending = write_count;

    mew_flash_mem_addr_receiving = (uint32_t) buffer_read;
    mew_flash_mem_size_receiving = read_count;

    __mew_spi_dma_send();

    if (op_type == MEW_FLASH_SYNC_OP) {
    	while (mew_flash_xfer_state != MEW_FLASH_XFER_STATE_FREE) {};
    }
}

void mew_spi_flash_set_on_receive_handler(void (*__mew_flash_rx_handler)(void)) {
	while (mew_flash_xfer_state != MEW_FLASH_XFER_STATE_FREE) {};
	mew_flash_rx_handler = __mew_flash_rx_handler;
}

unsigned int mew_spi_flash_handler(void) {
	if (mew_flash_rx_handler != NULL && mew_flash_xfer_state == MEW_FLASH_XFER_STATE_FREE) {
		mew_flash_rx_handler();
		mew_flash_rx_handler = NULL;
	}
	return 0;
}

uint32_t mew_spi_flash_get_async_state(void) {
	return mew_flash_xfer_state == MEW_FLASH_XFER_STATE_FREE ? 0 : 1;
}

