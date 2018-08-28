#include "display.h" 
#include "../debug/debug.h"

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
static void __mew_display_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);

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
    //__mew_unselect();
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
    //__mew_unselect();
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
    //spi_set_bidirectional_transmit_only_mode(DISPLAY_SPI);
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
    //spi_set_unidirectional_mode(DISPLAY_SPI);
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

static void __mew_spi_write_dma(const lv_color_t* buffer, uint16_t count) {
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

void mew_display_init(void) {
    __mew_init_control_pins();
    __mew_init_spi_pins();
    __mew_spi_init_non_dma();
    __mew_display_init();
    
    lv_init();

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.disp_flush = __mew_display_flush;
    lv_disp_drv_register(&disp_drv);

    lv_theme_t * th = lv_theme_material_init(250, &lv_font_dejavu_20);
    lv_theme_set_current(th);
}

static void __mew_display_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p) {
    while (_mew_dma_rx_state == 1) __asm__("NOP");
    
    __mew_spi_init_non_dma();
    __mew_display_write_cmd_with_data(0x2A, (uint16_t[]) { (x1 >> 8), x1, (x2 >> 8), x2 }, 4);
    __mew_display_write_cmd_with_data(0x2B, (uint16_t[]) { (y1 >> 8), y1, (y2 >> 8), y2 }, 4);
    __mew_display_write_cmd(0x2c);
    __mew_data();
    
    _mew_dma_rx_state = 1;
    __mew_spi_init_dma();
    __mew_spi_write_dma(color_p, ((y2 - y1 + 1) * (x2 - x1 + 1)));
}

void MEW_DISPLAY_DMA_HANDLE(void) {
    dma_stream_reset(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    dma_disable_stream(MEW_DISPLAY_DMA, MEW_DISPLAY_DMA_STREAM);
    
    _mew_dma_rx_state = 0;
    __mew_unselect();
    lv_flush_ready();

    //mew_debug_print("DISPLAY_DMA_HANDLE");
}

















//
//
//static void __mew_wait_fault_handler(void);
//static void __mew_wait_for_unlock(void);
//static void __mew_lock(void);
//static void __mew_unlock(void);
//static void __mew_start_spi_non_dma(void);
//static void __mew_start_spi_dma(void);
//static void __mew_spi_write_dma_i(const lv_color_t* buffer, uint16_t count);
//
//static volatile uint32_t _mew_spi_state = MEW_SPI_STATE_NULL;
//
//static void __mew_wait_fault_handler(void) {
//    mew_debug_print("ERROR: spi.c || Wait too long...");
//    __asm__("cpsid i");
//    while(1) {
//        __asm__("NOP");
//    }
//}
//
//uint32_t mew_is_spi_dma_bisy(void) {
//    return _mew_spi_state & MEW_SPI_STATE_BISY;
//}
//
//static void __mew_wait_for_unlock(void) {
//    uint32_t counter = 0;
//    while (_mew_spi_state & MEW_SPI_STATE_BISY) {
//        counter++;
//        if (counter > 50000000L) {
//            __mew_wait_fault_handler();
//        }
//    }
//}
//
//static void __mew_lock(void) {
//    _mew_spi_state |= MEW_SPI_STATE_BISY;
//}
//
//static void __mew_unlock(void) {
//    _mew_spi_state &= !MEW_SPI_STATE_BISY;
//}
//
//void __mew_start_spi_pins(void) {
//    gpio_mode_setup(DISPLAY_SPI_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, DISPLAY_SPI_GPIO_PINS);
//    gpio_set_output_options(DISPLAY_SPI_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, DISPLAY_SPI_GPIO_PINS);
//    gpio_set_af(DISPLAY_SPI_GPIO_PORT, DISPLAY_SPI_GPIO_AF_NUMBER, DISPLAY_SPI_GPIO_PINS);
//}
//
//static void __mew_start_spi_non_dma(void) { 
//    __mew_wait_for_unlock();
//    
//    if (!(_mew_spi_state & MEW_SPI_STATE_NONDMA)) {
//        __mew_lock();
//        
//        nvic_disable_irq(DISPLAY_DMA_NVIC);
//        spi_disable(DISPLAY_SPI);
//        spi_set_master_mode(DISPLAY_SPI);
//        spi_set_baudrate_prescaler(DISPLAY_SPI, SPI_CR1_BR_FPCLK_DIV_4);
//        spi_set_clock_polarity_0(DISPLAY_SPI);
//        spi_set_clock_phase_0(DISPLAY_SPI);
//        //spi_set_unidirectional_mode(DISPLAY_SPI);
//        spi_set_bidirectional_transmit_only_mode(DISPLAY_SPI);
//        spi_enable_software_slave_management(DISPLAY_SPI);
//        spi_send_msb_first(DISPLAY_SPI);
//        spi_set_nss_high(DISPLAY_SPI);
//        SPI_I2SCFGR(DISPLAY_SPI) &= ~SPI_I2SCFGR_I2SMOD;
//        spi_disable_tx_buffer_empty_interrupt(DISPLAY_SPI);
//        spi_disable_rx_buffer_not_empty_interrupt(DISPLAY_SPI);
//        spi_disable_error_interrupt(DISPLAY_SPI);
//        spi_disable_tx_dma(DISPLAY_SPI);
//        spi_disable_rx_dma(DISPLAY_SPI);
//        spi_set_dff_8bit(DISPLAY_SPI);
//        spi_send_msb_first(DISPLAY_SPI);
//        spi_enable(DISPLAY_SPI);
//        
// /*       spi_disable(DISPLAY_SPI);
//        spi_set_master_mode(DISPLAY_SPI);
//        spi_set_baudrate_prescaler(DISPLAY_SPI, SPI_CR1_BR_FPCLK_DIV_8);
//        spi_set_clock_polarity_0(DISPLAY_SPI);
//        spi_set_clock_phase_0(DISPLAY_SPI);
//        //spi_set_unidirectional_mode(DISPLAY_SPI);
//        spi_set_bidirectional_transmit_only_mode(DISPLAY_SPI);
//        spi_enable_software_slave_management(DISPLAY_SPI);
//        //spi_send_msb_first(DISPLAY_SPI);
//        spi_set_nss_high(DISPLAY_SPI);
//        //SPI_I2SCFGR(DISPLAY_SPI) &= ~SPI_I2SCFGR_I2SMOD;
//        spi_disable_tx_buffer_empty_interrupt(DISPLAY_SPI);
//        spi_disable_rx_buffer_not_empty_interrupt(DISPLAY_SPI);
//        spi_disable_error_interrupt(DISPLAY_SPI);
//        spi_disable_tx_dma(DISPLAY_SPI);
//        spi_disable_rx_dma(DISPLAY_SPI);
//        spi_set_dff_8bit(DISPLAY_SPI);
//        spi_send_msb_first(DISPLAY_SPI);
//        spi_enable(DISPLAY_SPI);
//        nvic_disable_irq(DISPLAY_DMA_NVIC);*/
//        
//        _mew_spi_state = MEW_SPI_STATE_NONDMA;
//    }
//}
//
//static void __mew_start_spi_dma(void) { 
//    __mew_wait_for_unlock();
//    
//    if (!(_mew_spi_state & MEW_SPI_STATE_DMA)) {
//        __mew_lock();
//        
//        spi_disable(DISPLAY_SPI);
//        spi_reset(DISPLAY_SPI);
//        //DISPLAY_SPI_I2SCFGR = 0;
//        spi_set_master_mode(DISPLAY_SPI);
//        spi_set_baudrate_prescaler(DISPLAY_SPI, SPI_CR1_BR_FPCLK_DIV_4);
//        spi_set_clock_polarity_0(DISPLAY_SPI);
//        spi_set_clock_phase_0(DISPLAY_SPI);
//        spi_set_unidirectional_mode(DISPLAY_SPI);
//        //spi_set_bidirectional_transmit_only_mode(DISPLAY_SPI);
//        spi_enable_software_slave_management(DISPLAY_SPI);
//        spi_set_nss_high(DISPLAY_SPI);
//        spi_disable_tx_buffer_empty_interrupt(DISPLAY_SPI);
//        spi_disable_rx_buffer_not_empty_interrupt(DISPLAY_SPI);
//        spi_disable_error_interrupt(DISPLAY_SPI);
//        spi_set_dff_16bit(DISPLAY_SPI);
//        spi_send_msb_first(DISPLAY_SPI);
//        spi_enable_tx_dma(DISPLAY_SPI);
//        //spi_disable_rx_dma(DISPLAY_SPI);
//        spi_enable(DISPLAY_SPI);
//        nvic_enable_irq(DISPLAY_DMA_NVIC);
//        
//        _mew_spi_state = MEW_SPI_STATE_DMA;
//    }
//}
//
//void __mew_spi_write_non_dma8(char* buffer, uint16_t size) {
//    uint32_t k;
//    //uint16_t data;
//    
//    __mew_wait_for_unlock();
//    __mew_start_spi_non_dma();
//    __mew_lock();
//        
//    for (k=0; k<size; k++) {
//        //data = buffer[k] & 0xFF;
//        spi_send(DISPLAY_SPI, buffer[k]);
//    }
//    
//   __mew_unlock();
//}
//
//void __mew_spi_write_non_dma16(uint16_t* buffer, uint16_t size) {
//    uint32_t k;
//    
//    __mew_wait_for_unlock();
//    __mew_start_spi_non_dma();
//    __mew_lock();
//        
//    for (k=0; k<size; k++) {
//        spi_send(DISPLAY_SPI, buffer[k]);
//    }
//    
//   __mew_unlock();
//}
//
//static void __mew_spi_write_dma_i(const lv_color_t* buffer, uint16_t count) {
//    mew_check_dma_memory((void*)buffer, "__mew_spi_write_dma_i");
//    
//    //dma_stream_reset(DISPLAY_DMA, DISPLAY_DMA_STREAM);
//    dma_set_transfer_mode(DISPLAY_DMA, DISPLAY_DMA_STREAM, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
//    dma_set_priority(DISPLAY_DMA,  DISPLAY_DMA_STREAM, DMA_SxCR_PL_VERY_HIGH);
//    dma_set_memory_size(DISPLAY_DMA, DISPLAY_DMA_STREAM, DMA_SxCR_MSIZE_16BIT);
//    dma_set_peripheral_size(DISPLAY_DMA, DISPLAY_DMA_STREAM, DMA_SxCR_PSIZE_16BIT);
//    dma_enable_memory_increment_mode(DISPLAY_DMA, DISPLAY_DMA_STREAM);
//    dma_set_peripheral_address(DISPLAY_DMA, DISPLAY_DMA_STREAM, (uint32_t) &DISPLAY_DMA_DR);
//    dma_channel_select(DISPLAY_DMA, DISPLAY_DMA_STREAM, DISPLAY_DMA_CHANNEL);
//    dma_set_memory_address(DISPLAY_DMA, DISPLAY_DMA_STREAM, (uint32_t) buffer);
//    dma_set_number_of_data(DISPLAY_DMA, DISPLAY_DMA_STREAM, count);
//    dma_disable_fifo_error_interrupt(DISPLAY_DMA, DISPLAY_DMA_STREAM);
//    dma_disable_half_transfer_interrupt(DISPLAY_DMA, DISPLAY_DMA_STREAM);
//    dma_enable_transfer_complete_interrupt(DISPLAY_DMA, DISPLAY_DMA_STREAM);
//    dma_enable_transfer_error_interrupt(DISPLAY_DMA,    DISPLAY_DMA_STREAM);
//    dma_enable_transfer_complete_interrupt(DISPLAY_DMA, DISPLAY_DMA_STREAM);
//    dma_enable_stream(DISPLAY_DMA, DISPLAY_DMA_STREAM);
//}
//
//void __mew_spi_write_dma(const lv_color_t* buffer, uint32_t count) {
//    if (count == 0) {
//        return;
//    }
//    
//    __mew_wait_for_unlock();
//    __mew_start_spi_dma();
//    __mew_lock();
//    
//    __mew_spi_write_dma_i(buffer, (uint16_t) count);
//}
//
//void mew_display_wait_dma_last_transfer(void) {
//    __mew_wait_for_unlock();
//}
//
//void DISPLAY_DMA_HANDLE(void) {
//    dma_stream_reset(DISPLAY_DMA, DISPLAY_DMA_STREAM);
//    dma_disable_stream(DISPLAY_DMA, DISPLAY_DMA_STREAM);
//
//    mew_debug_print("DISPLAY_DMA_HANDLE");
//    __mew_unlock();
//}
