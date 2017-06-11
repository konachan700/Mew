#include "board.h" 

static volatile u8 dma1_spi2_tx_complete = 1;

static volatile u8 dma1_i2c1_rx_complete = 1;
static volatile u8 dma1_i2c1_tx_complete = 1;
static volatile u8 dma1_i2c1_last_error = 0;

static volatile u32 last_random_value = 0;

struct dma1_i2c1_transaction* i2c_current_tr;

volatile u8 spi2_mode = 0;

#ifdef LIGHT_UI_THEME
    u8 buffer_for_fill_bg[1] = {0xFF};
#else
    u8 buffer_for_fill_bg[1] = {0};
#endif

const struct rcc_clock_scale rcc_hse_8mhz_3v3_96MHz = {
    /* 96MHz */
    .pllm = 4,
    .plln = 48,
    .pllp = 2,
    .pllq = 2,
    .hpre = RCC_CFGR_HPRE_DIV_NONE,
    .ppre1 = RCC_CFGR_PPRE_DIV_2,
    .ppre2 = RCC_CFGR_PPRE_DIV_NONE,
    .flash_config = FLASH_ACR_ICE | FLASH_ACR_DCE | FLASH_ACR_LATENCY_3WS,
    .apb1_frequency = 24000000,
    .apb2_frequency = 48000000,
};

static inline void __disable_irq(void)  { asm volatile("cpsid i"); }
static inline void __enable_irq(void)   { asm volatile("cpsie i"); }

u32 crc_gen(u32* data, u16 len) {
    crc_reset();
    return crc_calculate_block(data, len);
}

void start_random(void) {
    RNG_CR |= RNG_CR_IE;
    RNG_CR |= RNG_CR_RNGEN;
}

u32 random_u32(void) {
    u32 random = last_random_value;
	while (last_random_value == random) {
		if (((RNG_SR & (RNG_SR_SEIS | RNG_SR_CEIS)) == 0) && ((RNG_SR & RNG_SR_DRDY) == 1)) random = RNG_DR;
	}
	
	last_random_value = random;
    return last_random_value;
}

void memset_random_u32(u32* data, u16 len) {
    for (u16 i=0; i<len; i++) data[i] = random_u32();
}

void start_i2c1(void) {
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8 | GPIO9);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO8 | GPIO9);
	gpio_set_af(GPIOB, GPIO_AF4, GPIO8 | GPIO9);
    
	i2c_peripheral_disable(I2C1);
	i2c_reset(I2C1);
    i2c_set_fast_mode(I2C1);
	i2c_set_dutycycle(I2C1, I2C_CCR_DUTY_DIV2);
	i2c_set_clock_frequency(I2C1, I2C_CR2_FREQ_24MHZ);
	i2c_set_ccr(I2C1, 120);
	i2c_set_trise(I2C1, 24);
    i2c_enable_interrupt(I2C1, I2C_CR2_ITERREN | I2C_CR2_ITEVTEN);
    i2c_peripheral_enable(I2C1);
    
    nvic_enable_irq(NVIC_I2C1_EV_IRQ);
    nvic_enable_irq(NVIC_I2C1_ER_IRQ);
    nvic_enable_irq(NVIC_DMA1_STREAM0_IRQ);
    nvic_enable_irq(NVIC_DMA1_STREAM6_IRQ);
}

void i2c1_ev_isr(void) {
    u32 sr1 = I2C_SR1(I2C1), sr2;
    
    if (sr1 & I2C_SR1_SB) {
        i2c_send_7bit_address(I2C1, i2c_current_tr->dev_addr, i2c_current_tr->read_write);
        I2C_SR1(I2C1) &= !I2C_SR1_SB;
        //debug_print("I2C EV: I2C_SR1_SB");
    }
    
    if (sr1 & I2C_SR1_ADDR) {
        sr2 = I2C_SR2(I2C1);
        if (sr2 == 0) __asm__("NOP"); // prevent optimize
        I2C_SR1(I2C1) &= !I2C_SR1_ADDR;
        
        if (i2c_current_tr->read_write == I2C_READ) i2c_enable_ack(I2C1);
            
        //debug_print("I2C EV: I2C_SR1_ADDR");
    }
    
    if (sr1 & I2C_SR1_BTF) {
        I2C_SR1(I2C1) &= !I2C_SR1_BTF;
        //debug_print("I2C EV: I2C_SR1_BTF");
    }
    
    if (sr1 & I2C_SR1_STOPF) {
        I2C_SR1(I2C1) &= !I2C_SR1_STOPF;
        //debug_print("I2C EV: I2C_SR1_STOPF");
    }
}

void i2c1_er_isr(void) {
    i2c_current_tr->i2c_sr1 = I2C_SR1(I2C1);
    i2c_current_tr->i2c_sr2 = I2C_SR2(I2C1);
    
    if (i2c_current_tr->i2c_sr1 & I2C_SR1_BERR) {
        debug_print("I2C ERR: I2C_SR1_BERR");
        i2c_current_tr->last_error = I2C_SR1_BERR;
        I2C_SR1(I2C1) &= !I2C_SR1_BERR;
    }
    
    if (i2c_current_tr->i2c_sr1 & I2C_SR1_AF) {
        debug_print("I2C ERR: I2C_SR1_AF");
        i2c_current_tr->last_error = I2C_SR1_AF;
        I2C_SR1(I2C1) &= !I2C_SR1_AF;
    }
    
    if (i2c_current_tr->i2c_sr1 & I2C_SR1_ARLO) {
        debug_print("I2C ERR: I2C_SR1_ARLO");
        i2c_current_tr->last_error = I2C_SR1_ARLO;
        I2C_SR1(I2C1) &= !I2C_SR1_ARLO;
    }
    
    if (i2c_current_tr->i2c_sr1 & I2C_SR1_OVR) {
        debug_print("I2C ERR: I2C_SR1_OVR");
        i2c_current_tr->last_error = I2C_SR1_OVR;
        I2C_SR1(I2C1) &= !I2C_SR1_OVR;
    }
    
    dma1_i2c1_last_error == 1;
}

u32 i2c_fram_write_dma(u8 page, u8 start_byte, u8* buffer, u16 count) {
    struct dma1_i2c1_transaction i2c_tr;
    i2c_current_tr = &i2c_tr;
    
    if (dma1_i2c1_last_error == 1) {
        start_i2c1();
        dma1_i2c1_last_error == 0;
    }

    i2c_tr.dev_addr          = (0xA0 >> 1) | (page & 0x07);
    i2c_tr.read_write        = I2C_WRITE;
    i2c_tr.main_buffer_count = 1;
    i2c_tr.main_buffer       = (u8[]) {start_byte};
    i2c_tr.last_error        = 0;

    dma_stream_reset(DMA1, DMA_STREAM6);
    dma_disable_stream(DMA1, DMA_STREAM6);
    dma_set_transfer_mode(DMA1, DMA_STREAM6, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(DMA1, DMA_STREAM6, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(DMA1, DMA_STREAM6, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(DMA1, DMA_STREAM6, DMA_SxCR_PSIZE_8BIT);
    dma_enable_memory_increment_mode(DMA1, DMA_STREAM6);;
    dma_set_peripheral_address(DMA1, DMA_STREAM6, (uint32_t) &I2C1_DR);
    dma_channel_select(DMA1, DMA_STREAM6, DMA_SxCR_CHSEL_1);
    dma_set_memory_address(DMA1, DMA_STREAM6, (u32) (i2c_tr.main_buffer));
    dma_set_number_of_data(DMA1, DMA_STREAM6, (u16) (i2c_tr.main_buffer_count));
    dma_set_memory_burst(DMA1, DMA_STREAM6, DMA_SxCR_MBURST_INCR4);
    dma_set_peripheral_burst(DMA1, DMA_STREAM6, DMA_SxCR_PBURST_INCR4);
    dma_disable_fifo_error_interrupt(DMA1, DMA_STREAM6);
    dma_disable_half_transfer_interrupt(DMA1, DMA_STREAM6);
    dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM6);
    dma_enable_transfer_error_interrupt(DMA1, DMA_STREAM6);
    dma_enable_stream(DMA1, DMA_STREAM6);
    
    dma1_i2c1_tx_complete = 0;
    
    i2c_disable_ack(I2C1);
    i2c_clear_dma_last_transfer(I2C1);
    i2c_enable_dma(I2C1);
    i2c_send_start(I2C1);    
    
    while (dma1_i2c1_tx_complete == 0) __asm__("NOP");
    
    //dma1_i2c1_btf_complete = 0;
    dma1_i2c1_tx_complete = 0;
    
    i2c_tr.dev_addr          = (0xA0 >> 1) | (page & 0x07);
    i2c_tr.read_write        = I2C_WRITE;
    i2c_tr.main_buffer_count = count;
    i2c_tr.main_buffer       = buffer;
    i2c_tr.last_error        = 0;
    
    dma_stream_reset(DMA1, DMA_STREAM6);
    dma_disable_stream(DMA1, DMA_STREAM6);
    dma_set_transfer_mode(DMA1, DMA_STREAM6, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(DMA1, DMA_STREAM6, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(DMA1, DMA_STREAM6, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(DMA1, DMA_STREAM6, DMA_SxCR_PSIZE_8BIT);
    dma_enable_memory_increment_mode(DMA1, DMA_STREAM6);;
    dma_set_peripheral_address(DMA1, DMA_STREAM6, (uint32_t) &I2C1_DR);
    dma_channel_select(DMA1, DMA_STREAM6, DMA_SxCR_CHSEL_1);
    dma_set_memory_address(DMA1, DMA_STREAM6, (u32) (i2c_tr.main_buffer));
    dma_set_number_of_data(DMA1, DMA_STREAM6, (u16) (i2c_tr.main_buffer_count));
    dma_set_memory_burst(DMA1, DMA_STREAM6, DMA_SxCR_MBURST_INCR4);
    dma_set_peripheral_burst(DMA1, DMA_STREAM6, DMA_SxCR_PBURST_INCR4);
    dma_disable_fifo_error_interrupt(DMA1, DMA_STREAM6);
    dma_disable_half_transfer_interrupt(DMA1, DMA_STREAM6);
    dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM6);
    dma_enable_transfer_error_interrupt(DMA1, DMA_STREAM6);
    dma_enable_stream(DMA1, DMA_STREAM6);
    
    while (dma1_i2c1_tx_complete == 0) __asm__("NOP"); 
    for (u16 i=0; i<1500; i++) __asm__("NOP"); // wait for last byte written. Without this delay last byte will be lost.

    i2c_send_stop(I2C1);

    for (u16 i=0; i<0xFFFF; i++) __asm__("NOP"); // wait for bus stopped. 

    i2c_disable_dma(I2C1);
    i2c_peripheral_disable(I2C1); // bug in eeprom fm24cl16 - can't set correct reading address after a write operation.
                                  // i2c reset operation (high SDA on some short time) is needed.
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_MODE_OUTPUT, GPIO8 | GPIO9);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO8 | GPIO9);
    gpio_set(GPIOB, GPIO8 | GPIO9);
    
    for (u16 i=0; i<5000; i++) __asm__("NOP");
    
    start_i2c1();
    
    return i2c_tr.last_error;
}

u32 i2c_fram_read_dma(u8 page, u8 start_byte, u8* buffer, u16 count) {
    struct dma1_i2c1_transaction i2c_tr;
    i2c_current_tr = &i2c_tr;
    
    if (dma1_i2c1_last_error == 1) {
        start_i2c1();
        dma1_i2c1_last_error == 0;
    }
    
    i2c_tr.dev_addr          = (0xA0 >> 1) | (page & 0x07);
    i2c_tr.read_write        = I2C_WRITE;
    i2c_tr.main_buffer_count = 1;
    i2c_tr.main_buffer       = (u8[]) {start_byte};
    i2c_tr.last_error        = 0;
    
    dma_stream_reset(DMA1, DMA_STREAM6);
    dma_disable_stream(DMA1, DMA_STREAM6);
    dma_set_transfer_mode(DMA1, DMA_STREAM6, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(DMA1, DMA_STREAM6, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(DMA1, DMA_STREAM6, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(DMA1, DMA_STREAM6, DMA_SxCR_PSIZE_8BIT);
    dma_enable_memory_increment_mode(DMA1, DMA_STREAM6);;
    dma_set_peripheral_address(DMA1, DMA_STREAM6, (uint32_t) &I2C1_DR);
    dma_channel_select(DMA1, DMA_STREAM6, DMA_SxCR_CHSEL_1);
    dma_set_memory_address(DMA1, DMA_STREAM6, (u32) (i2c_tr.main_buffer));
    dma_set_number_of_data(DMA1, DMA_STREAM6, (uint16_t) (i2c_tr.main_buffer_count));
    dma_disable_fifo_error_interrupt(DMA1, DMA_STREAM6);
    dma_disable_half_transfer_interrupt(DMA1, DMA_STREAM6);
    dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM6);
    dma_enable_transfer_error_interrupt(DMA1, DMA_STREAM6);
    dma_enable_stream(DMA1, DMA_STREAM6);
    
    dma1_i2c1_tx_complete = 0;
    
    i2c_disable_ack(I2C1);
    i2c_clear_dma_last_transfer(I2C1);
    i2c_enable_dma(I2C1);
    i2c_send_start(I2C1);    
    
    while (dma1_i2c1_tx_complete == 0) __asm__("NOP");
    
    i2c_tr.dev_addr          = (0xA0 >> 1) | (page & 0x07);
    i2c_tr.read_write        = I2C_READ;
    i2c_tr.main_buffer_count = count;
    i2c_tr.main_buffer       = buffer;
    i2c_tr.last_error        = 0;
    
    dma_stream_reset(DMA1, DMA_STREAM0);
    dma_disable_stream(DMA1, DMA_STREAM0);
    dma_set_transfer_mode(DMA1, DMA_STREAM0, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
    dma_set_priority(DMA1, DMA_STREAM0, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(DMA1, DMA_STREAM0, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(DMA1, DMA_STREAM0, DMA_SxCR_PSIZE_8BIT);
    dma_enable_memory_increment_mode(DMA1, DMA_STREAM0);
    dma_set_peripheral_address(DMA1, DMA_STREAM0, (uint32_t) &I2C1_DR);
    dma_channel_select(DMA1, DMA_STREAM0, DMA_SxCR_CHSEL_1);
    dma_set_memory_address(DMA1, DMA_STREAM0, (u32) (i2c_tr.main_buffer));
    dma_set_number_of_data(DMA1, DMA_STREAM0, (u16) (i2c_tr.main_buffer_count));
    dma_disable_fifo_error_interrupt(DMA1, DMA_STREAM0);
    dma_disable_half_transfer_interrupt(DMA1, DMA_STREAM0);
    dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM0);
    dma_enable_transfer_error_interrupt(DMA1, DMA_STREAM0);
    dma_enable_stream(DMA1, DMA_STREAM0);
    
    dma1_i2c1_rx_complete = 0;
    
    i2c_set_dma_last_transfer(I2C1);
    i2c_send_start(I2C1);
    
    while (dma1_i2c1_rx_complete == 0) __asm__("NOP");
    
    i2c_send_stop(I2C1);
    i2c_disable_dma(I2C1);

    return i2c_tr.last_error;
}

void dma1_stream0_isr(void) {
    dma_stream_reset(DMA1, DMA_STREAM0);
    dma_disable_stream(DMA1, DMA_STREAM0);
    dma1_i2c1_rx_complete = 1;
}

void dma1_stream6_isr(void) {
    dma_stream_reset(DMA1, DMA_STREAM6);
    dma_disable_stream(DMA1, DMA_STREAM6);
    dma1_i2c1_tx_complete = 1;
}

void start_leds(void) {
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_MODE_OUTPUT, GPIO3);
    gpio_set(GPIOC, GPIO3);
}

void start_spi_2_non_dma(void) { 
    spi2_mode = 0;
    spi_disable(SPI2);
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO13 | GPIO14 | GPIO15);
	gpio_set_af(GPIOB, GPIO_AF5, GPIO13 | GPIO14 | GPIO15);
    spi_set_master_mode(SPI2);
    spi_set_baudrate_prescaler(SPI2, SPI_CR1_BR_FPCLK_DIV_64);
    spi_set_clock_polarity_0(SPI2);
    spi_set_clock_phase_0(SPI2);
    spi_set_unidirectional_mode(SPI2);
    spi_enable_software_slave_management(SPI2);
    spi_send_msb_first(SPI2);
    spi_set_nss_high(SPI2);
    SPI_I2SCFGR(SPI2) &= ~SPI_I2SCFGR_I2SMOD;
    spi_disable_tx_buffer_empty_interrupt(SPI2);
    spi_disable_rx_buffer_not_empty_interrupt(SPI2);
    spi_disable_error_interrupt(SPI2);
    spi_disable_tx_dma(SPI2);
    spi_disable_rx_dma(SPI2);
    spi2_mode = 1;
    spi_enable(SPI2);
}

void start_spi_2_dma(void) {  
    spi2_mode = 0;
    spi_reset(SPI2);
    SPI2_I2SCFGR = 0;
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO13 | GPIO14 | GPIO15);
	gpio_set_af(GPIOB, GPIO_AF5, GPIO13 | GPIO14 | GPIO15);
    spi_set_master_mode(SPI2);
    spi_set_baudrate_prescaler(SPI2, SPI_CR1_BR_FPCLK_DIV_2);
    spi_set_clock_polarity_0(SPI2);
    spi_set_clock_phase_0(SPI2);
    spi_set_unidirectional_mode(SPI2);
    spi_enable_software_slave_management(SPI2);
    spi_send_msb_first(SPI2);
    spi_set_nss_high(SPI2);
    spi_set_dff_8bit(SPI2);
    spi_disable_tx_buffer_empty_interrupt(SPI2);
    spi_disable_rx_buffer_not_empty_interrupt(SPI2);
    spi_disable_error_interrupt(SPI2);
    spi_enable_tx_dma(SPI2);
    spi_enable_rx_dma(SPI2);
    spi_enable(SPI2);
    nvic_enable_irq(NVIC_DMA1_STREAM4_IRQ);
    spi2_mode = 2;
}

void spi_xfer_dma(u8* buf, u16 count, u8 is_blocking) {
    if (spi2_mode == 2) {        
        while (dma1_spi2_tx_complete == 0) __asm__("NOP");
        //while ((DMA1_SCR(DMA_STREAM4) & DMA_SxCR_EN) != 0);
        
        dma_stream_reset(DMA1, DMA_STREAM4);
        dma_set_transfer_mode(DMA1, DMA_STREAM4, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
        dma_set_priority(DMA1,  DMA_STREAM4, DMA_SxCR_PL_HIGH);
        dma_set_memory_size(DMA1, DMA_STREAM4, DMA_SxCR_MSIZE_8BIT);
        dma_set_peripheral_size(DMA1, DMA_STREAM4, DMA_SxCR_PSIZE_8BIT);
        
        if (buf != NULL) 
            dma_enable_memory_increment_mode(DMA1, DMA_STREAM4);
        else
            dma_disable_memory_increment_mode(DMA1, DMA_STREAM4);
        
        dma_set_peripheral_address(DMA1, DMA_STREAM4, (uint32_t) &SPI2_DR);
        dma_channel_select(DMA1, DMA_STREAM4, DMA_SxCR_CHSEL_0);
        
        if (buf != NULL) 
            dma_set_memory_address(DMA1, DMA_STREAM4, (uint32_t) buf);
        else 
            dma_set_memory_address(DMA1, DMA_STREAM4, (uint32_t) buffer_for_fill_bg);
        
        dma_set_number_of_data(DMA1, DMA_STREAM4, count);
        dma_disable_fifo_error_interrupt(DMA1, DMA_STREAM4);
        dma_disable_half_transfer_interrupt(DMA1, DMA_STREAM4);
        dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM4);
        dma_enable_transfer_error_interrupt(DMA1,    DMA_STREAM4);
        dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM4);
        dma_enable_stream(DMA1, DMA_STREAM4);
        dma1_spi2_tx_complete = 0;
        
        if (is_blocking == 1)
            while (dma1_spi2_tx_complete == 0) __asm__("NOP");
            
    } else if (spi2_mode == 1) {
        u8 i;
        u16 j;
        for (i=0; i<count; i++) {
            j = SPI_DR(SPI2);
            spi_send(SPI2, buf[i]);
            if (j == 0) __asm__("NOP");
            while (!(SPI_SR(SPI2) & SPI_SR_RXNE));
        }
    }
}

void dma1_stream4_isr(void) {
    dma_stream_reset(DMA1, DMA_STREAM4);
    dma_disable_stream(DMA1, DMA_STREAM4);
    dma1_spi2_tx_complete = 1;
}

void start_buttons(void) {
    gpio_mode_setup(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO12 | GPIO13 | GPIO14 | GPIO15);
    gpio_set(GPIOE, GPIO12 | GPIO13 | GPIO14 | GPIO15);
    nvic_enable_irq(NVIC_EXTI15_10_IRQ);
    exti_select_source(EXTI12 | EXTI13 | EXTI14 | EXTI15, GPIOE);
	exti_set_trigger(EXTI12 | EXTI13 | EXTI14 | EXTI15, EXTI_TRIGGER_FALLING);
	exti_enable_request(EXTI12 | EXTI13 | EXTI14 | EXTI15);
}

void start_backlight(void) {
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_MODE_OUTPUT, GPIO6);
    gpio_set(GPIOA, GPIO6);
}

void start_debug_usart(void) {
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO2 | GPIO3);
    //USART_BRR(USART2) = 104;
    usart_set_baudrate(USART2, 115200);
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART2, USART_MODE_TX);
    usart_enable(USART2);
    printf("test");
}

void debug_print(u8* text) {
    u16 i = 0;
    while (1) {
        if (text[i] == 0) {
            usart_send_blocking(USART2, '\r');
            usart_send_blocking(USART2, '\n');
            return;
        }
        usart_send_blocking(USART2, text[i]);
        i++;
    }
}

u8 __to_hex(u8 in) {
	char buf = in & 0x0F;
	switch (buf) {
	case 0:
		return '0';
	case 1:
		return '1';
	case 2:
		return '2';
	case 3:
		return '3';
	case 4:
		return '4';
	case 5:
		return '5';
	case 6:
		return '6';
	case 7:
		return '7';
	case 8:
		return '8';
	case 9:
		return '9';
	case 0x0A:
		return 'A';
	case 0x0B:
		return 'B';
	case 0x0C:
		return 'C';
	case 0x0D:
		return 'D';
	case 0x0E:
		return 'E';
	case 0x0F:
		return 'F';
	}
	return '0';
}

void debug_print_hex(u8* blob, u16 len) {
    u16 i = 0, nl = 0;
    for (i=0; i<len; i++) {
        usart_send_blocking(USART2, __to_hex(blob[i] >> 4));
        usart_send_blocking(USART2, __to_hex(blob[i]));
        usart_send_blocking(USART2, ' ');
        
        nl++;
        if (nl >= 16) {
            nl = 0;
            usart_send_blocking(USART2, '\r');
            usart_send_blocking(USART2, '\n');
        }
    }
        
    usart_send_blocking(USART2, '\r');
    usart_send_blocking(USART2, '\n');
}

void start_all_clock(void) {
    rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3_96MHz);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_GPIOE);
    
    rcc_periph_clock_enable(RCC_DMA1);
    rcc_periph_clock_enable(RCC_DMA2);
    
    rcc_periph_clock_enable(RCC_SYSCFG);
    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_USART2);
    rcc_periph_clock_enable(RCC_SPI2);
    rcc_periph_clock_enable(RCC_OTGFS);
    rcc_periph_clock_enable(RCC_I2C1);
    rcc_periph_clock_enable(RCC_SDIO);
    rcc_periph_clock_enable(RCC_RNG);
    rcc_periph_clock_enable(RCC_CRYP);
    rcc_periph_clock_enable(RCC_CRC);
}

void start_timer_2(void) {
	nvic_enable_irq(NVIC_TIM2_IRQ);
	nvic_set_priority(NVIC_TIM2_IRQ, 4);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM2, 1200);
	timer_disable_preload(TIM2);
	timer_continuous_mode(TIM2);
	timer_set_period(TIM2, 1000);
	timer_disable_oc_output(TIM2, TIM_OC1);
	timer_disable_oc_output(TIM2, TIM_OC2);
	timer_disable_oc_output(TIM2, TIM_OC3);
	timer_disable_oc_output(TIM2, TIM_OC4);
	timer_disable_oc_clear(TIM2, TIM_OC1);
	timer_disable_oc_preload(TIM2, TIM_OC1);
	timer_set_oc_slow_mode(TIM2, TIM_OC1);
	timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_FROZEN);
	timer_set_oc_value(TIM2, TIM_OC1, 1000);
	timer_disable_preload(TIM2);
	timer_enable_counter(TIM2);
	timer_enable_irq(TIM2, TIM_DIER_CC1IE);    
}
