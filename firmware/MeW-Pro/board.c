#include "board.h" 

volatile u8 dma1_spi2_tx_complete = 1;

volatile u8 dma1_i2c1_rx_complete = 1;
volatile u8 dma1_i2c1_tx_complete = 1;
volatile u8 dma1_i2c1_last_error = 0;

volatile u8 dma2_sdio_complete = 1;

struct dma1_i2c1_transaction* i2c_current_tr;

volatile u8 spi2_mode = 0;

#ifdef LIGHT_UI_THEME
    u8 buffer_for_fill_bg[1] = {0xFF};
#else
    u8 buffer_for_fill_bg[1] = {0};
#endif
    
volatile u32 sdio_status_ccs = 0;
volatile u32 sdio_status_rca = 0;
volatile u32 sdio_card_size  = 0;

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

u32 __sdio_get_cmd_result(void) {
    u32 status = SDIO_STA & 0xFFF;

    if (status & SDIO_STA_CMDACT) return SDIO_EINPROGRESS;

    if (status & SDIO_STA_CTIMEOUT) {
        SDIO_ICR = SDIO_STA_CTIMEOUT;
        return SDIO_ECTIMEOUT;
    }
    
    if (status & SDIO_STA_CMDREND) {
        SDIO_ICR = SDIO_STA_CMDREND;
        return SDIO_ESUCCESS;
    }

    if (status & SDIO_STA_CMDSENT) {
        SDIO_ICR = SDIO_STA_CMDSENT;
        return SDIO_ESUCCESS;
    }

    if (status & SDIO_STA_CCRCFAIL) {
        SDIO_ICR = SDIO_STA_CCRCFAIL;
        return SDIO_ECCRCFAIL;
    }

    return SDIO_EUNKNOWN;
}

void __sdio_cmd(uint32_t cmd, uint32_t arg) {
    cmd &= SDIO_CMD_CMDINDEX_MSK;
    u32 res = SDIO_CMD_WAITRESP_SHORT;
    
    if (cmd == 0) 
        res = SDIO_CMD_WAITRESP_NO_0;
     else if (cmd == 2 || cmd == 9 || cmd == 10) 
        res = SDIO_CMD_WAITRESP_LONG;

    SDIO_ICR = 0x7ff;
    SDIO_ARG = arg; 
    SDIO_CMD = (cmd | SDIO_CMD_CPSMEN | res);
}

u32 __sdio_cmd_wait(u32 cmd, u32 arg) {
    u32 res;
    __sdio_cmd(cmd, arg);
    while ((res = __sdio_get_cmd_result()) == SDIO_EINPROGRESS) __asm__("NOP");
    return res;
}

u32 __sdio_cmd_retry(uint32_t cmd, uint32_t arg) {
    u32 res;
    for (u16 i=0; i<SDIO_MAX_CMD_RETRIES; i++) {
        res = __sdio_cmd_wait(cmd, arg);
        if (res == SDIO_ESUCCESS) break;
    }
    return res;
}

u32 __sdio_cmd_app(uint32_t cmd, uint32_t arg) {
    u32 res, 
        eres = (cmd == 41) ? SDIO_ECCRCFAIL : SDIO_ESUCCESS;
    
    for (u16 i=0; i<SDIO_MAX_CMD_RETRIES; i++) {
        res = __sdio_cmd_wait(55, sdio_status_rca << 16);
        if (res != SDIO_ESUCCESS) continue;
        
        res = __sdio_cmd_wait(cmd, arg);
        if (res == eres) break;
    }
    return res;
}

u32 __sdio_wakeup(void) {
    if (__sdio_cmd_retry(SDIO_CMD_GO_IDLE_STATE, 0) != SDIO_ESUCCESS) {
        SDIO_POWER = SDIO_POWER_PWRCTRL_PWROFF;
        debug_print("SDIO ERR: CMD 0");
        return 1;
    }
    return 0;
}

void start_sdio(void) {
    u32 res, timer, sdioresp = 0;
    u8  hcs         = 0, 
        cmdstat     = 0;
    
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO1);
    gpio_set(GPIOA, GPIO1);
    
    gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
    gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO2);
	gpio_set_af(GPIOD, GPIO_AF12, GPIO2);
    
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12);
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12);
	gpio_set_af(GPIOC, GPIO_AF12, GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12);
    
    SDIO_POWER = SDIO_POWER_PWRCTRL_PWRON;
    while (SDIO_POWER != SDIO_POWER_PWRCTRL_PWRON);
    SDIO_CLKCR = SDIO_CLKCR_CLKEN | 0x76;
    
    if (__sdio_wakeup() != 0) return;
    
    res = __sdio_cmd_retry(8, 0x1AA);
    if ((res == SDIO_ESUCCESS) && (SDIO_RESP1 == 0x1AA))
        hcs = 1;
    else if (res == SDIO_ECTIMEOUT) {
        hcs = 0;
        debug_print("SDIO DEBUG: CMD 8 TIMEOUT");
    } else {
        SDIO_POWER = SDIO_POWER_PWRCTRL_PWROFF;
        debug_print("SDIO ERR: CMD 8");
        return;
    }
    
    timer = 0;
    cmdstat = 0;
    while (timer < SDIO_CMD41_TIMER) {
        timer++;
        res = __sdio_cmd_app(SDIO_ACMD_SEND_OP_COND, 0x100000 | ((hcs == 1) ? SDIO_OCR_HCS : 0));
        sdioresp = SDIO_RESP1;
        if ((res == SDIO_ECCRCFAIL) && ((sdioresp & SDIO_OCR_BUSY) != 0)) {
            sdio_status_ccs = ((sdioresp & SDIO_OCR_CCS) != 0) ? 1 : 0;
            cmdstat = 1;
            break;
        }
    }
    
    if (cmdstat == 0) {
        SDIO_POWER = SDIO_POWER_PWRCTRL_PWROFF;
        debug_print("SDIO ERR: CMD 41");
        return;
    }
    
    if (__sdio_cmd_retry(2, 0) != SDIO_ESUCCESS) {
        SDIO_POWER = SDIO_POWER_PWRCTRL_PWROFF;
        debug_print("SDIO ERR: CMD 2");
        return;
    }
    
    cmdstat = 0;
    for (int i=0; i<SDIO_MAX_CMD_RETRIES; i++) {
        if (__sdio_cmd_wait(3, 0) == SDIO_ESUCCESS) {
            sdioresp = SDIO_RESP1;
            sdio_status_rca = sdioresp >> 16;
            if (sdio_status_rca != 0) {
                cmdstat = 1;
                break;
            }
        }
    }
    
    if (cmdstat == 0) {
        SDIO_POWER = SDIO_POWER_PWRCTRL_PWROFF;
        debug_print("SDIO ERR: CMD 3");
        return;
    }
    
    if (__sdio_cmd_retry(9, sdio_status_rca << 16) != SDIO_ESUCCESS) {
        SDIO_POWER = SDIO_POWER_PWRCTRL_PWROFF;
        debug_print("SDIO ERR: CMD 9");
        return;
    }

    u32 csd_version = SDIO_RESP1 >> 30;
    if (csd_version == 0)
        sdio_card_size = (SDIO_CSD_V1_BLOCK_LEN * SDIO_CSD_V1_BLOCK_NR) >> 9;
    else if (csd_version == 1) 
        sdio_card_size = (SDIO_CSD_V2_C_SIZE + 1) << 10;
    else {
        SDIO_POWER = SDIO_POWER_PWRCTRL_PWROFF;
        debug_print("SDIO ERR: CMD 3");
        return;
    }

    if (__sdio_cmd_retry(7, sdio_status_rca << 16) != SDIO_ESUCCESS) {
        SDIO_POWER = SDIO_POWER_PWRCTRL_PWROFF;
        debug_print("SDIO ERR: CMD 7. SELECT");
        return;
    }

    if (__sdio_cmd_app(6, 2) != SDIO_ESUCCESS) {
        SDIO_POWER = SDIO_POWER_PWRCTRL_PWROFF;
        debug_print("SDIO ERR: CMD 6. 4-BIT MODE");
        return;
    }

    SDIO_CLKCR = SDIO_CLKCR_CLKEN | SDIO_CLKCR_WIDBUS_4;
    debug_print("SDIO OK!");
    debug_print_hex((u8*) &sdio_card_size, 4);
}

u32 __sdio_wait_dr(void) {
    u32 timer = 0;
    while (timer < SDIO_CMD41_TIMER) {
        timer++;
        if ((__sdio_cmd_wait(13, sdio_status_rca << 16) == SDIO_ESUCCESS) && ((SDIO_RESP1 & 0x100) != 0)) return SDIO_OK;
    }
    return SDIO_GENERAL_ERROR;
}

u32 sdio_read_b512(u32 address, u32 *buffer) {
    if (__sdio_wait_dr() == SDIO_GENERAL_ERROR) {
        return SDIO_GENERAL_ERROR;
    }

    if (sdio_status_ccs == 0) {
        address *= 512;
        if (__sdio_cmd_retry(16, 512) != SDIO_ESUCCESS) {
            debug_print("SDIO ERR RD: CMD 16");
            return SDIO_GENERAL_ERROR;
        }
    }

    dma2_sdio_complete = 0;
    
    dma_stream_reset(DMA2, DMA_STREAM3);
    nvic_enable_irq(NVIC_DMA2_STREAM3_IRQ);
    dma_set_transfer_mode(DMA2, DMA_STREAM3, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
    dma_set_priority(DMA2,  DMA_STREAM3, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(DMA2, DMA_STREAM3, DMA_SxCR_MSIZE_32BIT);
    dma_set_peripheral_size(DMA2, DMA_STREAM3, DMA_SxCR_PSIZE_32BIT);
    dma_enable_memory_increment_mode(DMA2, DMA_STREAM3);
    dma_set_peripheral_address(DMA2, DMA_STREAM3, (u32) &SDIO_FIFO);
    dma_channel_select(DMA2, DMA_STREAM3, DMA_SxCR_CHSEL_4);
    dma_set_memory_address(DMA2, DMA_STREAM3, (u32) buffer);
    dma_set_number_of_data(DMA2, DMA_STREAM3, 128);
    dma_disable_fifo_error_interrupt(DMA2, DMA_STREAM3);
    dma_disable_half_transfer_interrupt(DMA2, DMA_STREAM3);
    dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM3);
    dma_enable_transfer_error_interrupt(DMA2,    DMA_STREAM3);
    dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM3);
    dma_enable_stream(DMA2, DMA_STREAM3);
    

    // A 100ms timeout expressed as ticks in the 24Mhz bus clock.
    //SDIO_DTIMER = 2400000;
    SDIO_DTIMER = 9600000;
    SDIO_DLEN = 512;
    SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_9 | SDIO_DCTRL_DMAEN | SDIO_DCTRL_DTDIR | SDIO_DCTRL_SDIOEN | SDIO_DCTRL_DTEN;

    if (__sdio_cmd_wait(17, address) != SDIO_ESUCCESS) {
        debug_print("SDIO ERR RD: CMD 17");
        return SDIO_GENERAL_ERROR;
    }

    while (true) {
        u32 result = SDIO_STA;
        if (result & (DATA_RX_SUCCESS_FLAGS | DATA_RX_ERROR_FLAGS)) {
            if (result & DATA_RX_ERROR_FLAGS) {
                debug_print("SDIO ERR RD: DATA_RX_ERROR_FLAGS");
                return SDIO_GENERAL_ERROR;
            }
            break;
        }
    }
    
    while (dma2_sdio_complete == 0) __asm__("NOP");

    return SDIO_OK;
}

void dma2_stream3_isr(void) {
    dma_stream_reset(DMA2, DMA_STREAM3);
    dma_disable_stream(DMA2, DMA_STREAM3);
    
    dma2_sdio_complete = 1;
    debug_print("dma2_stream3_isr");
}

void start_i2c1(void) {
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8 | GPIO9);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO8 | GPIO9);
	gpio_set_af(GPIOB, GPIO_AF4, GPIO8 | GPIO9);
    
	i2c_peripheral_disable(I2C1);
	i2c_reset(I2C1);
	i2c_set_standard_mode(I2C1);
	i2c_enable_ack(I2C1);
	i2c_set_dutycycle(I2C1, I2C_CCR_DUTY_DIV2);
	i2c_set_clock_frequency(I2C1, I2C_CR2_FREQ_24MHZ);
	i2c_set_ccr(I2C1, 210);
	i2c_set_trise(I2C1, 43);
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
        //debug_print("I2C EV: I2C_SR1_ADDR");
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
    
    dma1_i2c1_tx_complete = 1;
    dma1_i2c1_rx_complete = 1;
    dma1_i2c1_last_error == 1;
    
    dma_stream_reset(DMA1, DMA_STREAM0);
    dma_disable_stream(DMA1, DMA_STREAM0);
    dma_stream_reset(DMA1, DMA_STREAM6);
    dma_disable_stream(DMA1, DMA_STREAM6);
    
    i2c_send_stop(I2C1);
    i2c_disable_dma(I2C1);
}

void i2c_dma_req(struct dma1_i2c1_transaction* i2c_tr) {
    u8 dma_ch = (i2c_tr->read_write == I2C_READ) ? DMA_STREAM0 : DMA_STREAM6;
    
    if (i2c_tr->read_write == I2C_READ)
        while (dma1_i2c1_rx_complete == 0) __asm__("NOP");
    else
        while (dma1_i2c1_tx_complete == 0) __asm__("NOP");
        
    i2c_current_tr = i2c_tr;
    if (dma1_i2c1_last_error == 1) {
        start_i2c1();
        dma1_i2c1_last_error == 0;
    }
    
    dma_stream_reset(DMA1, dma_ch);
    
    if (i2c_tr->read_write == I2C_WRITE)    
        dma_set_transfer_mode(DMA1, dma_ch, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    else 
        dma_set_transfer_mode(DMA1, dma_ch, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
    
    dma_set_priority(DMA1,  dma_ch, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(DMA1, dma_ch, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(DMA1, dma_ch, DMA_SxCR_PSIZE_8BIT);
    dma_enable_memory_increment_mode(DMA1, dma_ch);;
    dma_set_peripheral_address(DMA1, dma_ch, (uint32_t) &I2C1_DR);
    dma_channel_select(DMA1, dma_ch, DMA_SxCR_CHSEL_1);
    dma_set_memory_address(DMA1, dma_ch, (uint32_t) i2c_tr->buffer);
    dma_set_number_of_data(DMA1, dma_ch, i2c_tr->buffer_count);
    dma_disable_fifo_error_interrupt(DMA1, dma_ch);
    dma_disable_half_transfer_interrupt(DMA1, dma_ch);
    
    dma_enable_transfer_complete_interrupt(DMA1, dma_ch);
    dma_enable_transfer_error_interrupt(DMA1, dma_ch);
    dma_enable_transfer_complete_interrupt(DMA1, dma_ch);
    dma_enable_stream(DMA1, dma_ch);
    
    i2c_enable_dma(I2C1);
    
    if (i2c_tr->read_write == I2C_READ) {
        i2c_set_dma_last_transfer(I2C1);
        i2c_enable_ack(I2C1);
        dma1_i2c1_rx_complete = 0;
    } else 
        dma1_i2c1_tx_complete = 0;
    
    i2c_send_start(I2C1);    
}

u32 i2c_fram_write_dma(u8 page, u8 start_byte, u8* buffer, u16 count) {
    struct dma1_i2c1_transaction i2c_tr;
    
    u8 buffer_z[count+1];
    memcpy(buffer_z+1, buffer, count);
    buffer_z[0] = start_byte;
    
    i2c_tr.dev_addr     = (0xA0 >> 1) | (page & 0x07);
    i2c_tr.read_write   = I2C_WRITE;
    i2c_tr.buffer_count = count+1;
    i2c_tr.buffer       = buffer_z;
    i2c_tr.last_error   = 0;
    i2c_dma_req(&i2c_tr);
    i2c_write_dma_wait();
    
    if (i2c_tr.last_error != 0) return i2c_tr.last_error;
    
    return 0;
}

u32 i2c_fram_read_dma(u8 page, u8 start_byte, u8* buffer, u16 count) {
    struct dma1_i2c1_transaction i2c_tr;
    u8 wr_buf[1] = {start_byte};
    
    i2c_tr.dev_addr     = (0xA0 >> 1) | (page & 0x07);
    i2c_tr.read_write   = I2C_WRITE;
    i2c_tr.buffer_count = 1;
    i2c_tr.buffer       = wr_buf;
    i2c_tr.last_error   = 0;
    i2c_dma_req(&i2c_tr);
    i2c_write_dma_wait();
    
    if (i2c_tr.last_error != 0) return i2c_tr.last_error;
    
    i2c_tr.dev_addr     = (0xA0 >> 1) | (page & 0x07);
    i2c_tr.read_write   = I2C_READ;
    i2c_tr.buffer_count = count;
    i2c_tr.buffer       = buffer;
    i2c_tr.last_error   = 0;
    i2c_dma_req(&i2c_tr);
    i2c_read_dma_wait();
    
    if (i2c_tr.last_error != 0) return i2c_tr.last_error;
    
    return 0;
}

void i2c_read_dma_wait(void) {
    while (dma1_i2c1_rx_complete == 0) __asm__("NOP");
}

void i2c_write_dma_wait(void) {
    while (dma1_i2c1_tx_complete == 0) __asm__("NOP");
}

void dma1_stream0_isr(void) {
    dma_stream_reset(DMA1, DMA_STREAM0);
    dma_disable_stream(DMA1, DMA_STREAM0);
    
    i2c_send_stop(I2C1);
    i2c_disable_dma(I2C1);
    
    dma1_i2c1_rx_complete = 1;
    //debug_print("dma1_stream0_isr");
}

void dma1_stream6_isr(void) {
    dma_stream_reset(DMA1, DMA_STREAM6);
    dma_disable_stream(DMA1, DMA_STREAM6);
    
    i2c_send_stop(I2C1);
    i2c_disable_dma(I2C1);

    dma1_i2c1_tx_complete = 1;
    //debug_print("dma1_stream6_isr");
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
    u16 i = 0;
    for (i=0; i<len; i++) {
        usart_send_blocking(USART2, __to_hex(blob[i] >> 4));
        usart_send_blocking(USART2, __to_hex(blob[i]));
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
}

void start_timer_2(void) {
	nvic_enable_irq(NVIC_TIM2_IRQ);
	nvic_set_priority(NVIC_TIM2_IRQ, 4);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM2, 2400);
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
