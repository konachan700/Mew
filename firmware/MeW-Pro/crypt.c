#include "crypt.h"  

struct security_information* sec_info;

volatile u8 dma2_crypt_in_complete = 1;
volatile u8 dma2_crypt_out_complete = 1;

static inline struct security_information* crypt_get_security_information(void) {
	return (struct security_information*)(FLASH_SI_GLOBAL_OFFSET);
}



u32 mewcrypt_aes256_gen_keys(void) {
    sec_info = crypt_get_security_information();
    if (sec_info->magic != MEW_SECURITY_INFO_MAGIC) {
        struct security_information sec_info_temp;
        sec_info_temp.magic = MEW_SECURITY_INFO_MAGIC;
        sec_info_temp.uid = random_u32();
        memset_random_u32((u32*) sec_info_temp.key, 32);
        memset_random_u32((u32*) sec_info_temp.iv, 16);
        
        flash_unlock();
        flash_erase_sector(7, 2);
        flash_program(FLASH_SI_GLOBAL_OFFSET, (u8*) &sec_info_temp, sizeof(struct security_information));
        flash_lock();
        debug_print("Magic is bad. New keys generated, all data was lost!");
    }
    
    if (sec_info->magic != MEW_SECURITY_INFO_MAGIC) {
        debug_print("Cannot program flash. Stopped.");
        while (1) __asm__("NOP");
    }
    
    return 0;
}

u32 mewcrypt_aes256(u8 dir, u32* in_block, u32* out_block, u32 count) {
    u8 i;
    
    dma2_crypt_in_complete = 0;
    nvic_enable_irq(NVIC_DMA2_STREAM6_IRQ);
    
    dma_disable_stream(DMA2, DMA_STREAM6);
    dma_stream_reset(DMA2, DMA_STREAM6);
    dma_set_transfer_mode(DMA2, DMA_STREAM6, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(DMA2,  DMA_STREAM6, DMA_SxCR_PL_LOW);
    dma_set_memory_size(DMA2, DMA_STREAM6, DMA_SxCR_MSIZE_32BIT);
    dma_set_peripheral_size(DMA2, DMA_STREAM6, DMA_SxCR_PSIZE_32BIT);
    dma_enable_memory_increment_mode(DMA2, DMA_STREAM6);
    dma_set_peripheral_address(DMA2, DMA_STREAM6, (u32) &CRYP_DIN);
    dma_channel_select(DMA2, DMA_STREAM6, DMA_SxCR_CHSEL_2);
    dma_set_memory_address(DMA2, DMA_STREAM6, (u32) in_block);
    dma_set_number_of_data(DMA2, DMA_STREAM6, count);
    dma_enable_fifo_mode(DMA2, DMA_STREAM6);
    dma_set_fifo_threshold(DMA2, DMA_STREAM6, DMA_SxFCR_FTH_4_4_FULL);
    dma_set_memory_burst(DMA2, DMA_STREAM6, DMA_SxCR_MBURST_INCR4);
    dma_set_peripheral_burst(DMA2, DMA_STREAM6, DMA_SxCR_PBURST_INCR4);
    dma_disable_fifo_error_interrupt(DMA2, DMA_STREAM6);
    dma_disable_half_transfer_interrupt(DMA2, DMA_STREAM6);
    dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM6);
    dma_enable_transfer_error_interrupt(DMA2,    DMA_STREAM6);
    dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM6);
    
    dma2_crypt_out_complete = 0;
    nvic_enable_irq(NVIC_DMA2_STREAM5_IRQ);
    
    dma_disable_stream(DMA2, DMA_STREAM5);
    dma_stream_reset(DMA2, DMA_STREAM5);
    dma_set_transfer_mode(DMA2, DMA_STREAM5, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
    dma_set_priority(DMA2,  DMA_STREAM5, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(DMA2, DMA_STREAM5, DMA_SxCR_MSIZE_32BIT);
    dma_set_peripheral_size(DMA2, DMA_STREAM5, DMA_SxCR_PSIZE_32BIT);
    dma_enable_memory_increment_mode(DMA2, DMA_STREAM5);
    dma_set_peripheral_address(DMA2, DMA_STREAM5, (u32) &CRYP_DOUT);
    dma_channel_select(DMA2, DMA_STREAM5, DMA_SxCR_CHSEL_2);
    dma_set_memory_address(DMA2, DMA_STREAM5, (u32) out_block);
    dma_set_number_of_data(DMA2, DMA_STREAM5, count);
    dma_enable_fifo_mode(DMA2, DMA_STREAM5);
    dma_set_fifo_threshold(DMA2, DMA_STREAM5, DMA_SxFCR_FTH_4_4_FULL);
    dma_set_memory_burst(DMA2, DMA_STREAM5, DMA_SxCR_MBURST_INCR4);
    dma_set_peripheral_burst(DMA2, DMA_STREAM5, DMA_SxCR_PBURST_INCR4);
    dma_disable_fifo_error_interrupt(DMA2, DMA_STREAM5);
    dma_disable_half_transfer_interrupt(DMA2, DMA_STREAM5);
    dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM5);
    dma_enable_transfer_error_interrupt(DMA2,    DMA_STREAM5);
    dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM5);

    while (CRYP_SR & CRYP_SR_BUSY) __asm__("NOP");
    CRYP_CR = CRYP_CR_KEYSIZE_256;
    for (i=0; i<4; i++) {
        CRYP_KR(i) = sec_info->key[i];
    } 

    if (dir == MEW_DECRYPT) {
        CRYP_CR |= CRYP_CR_ALGOMODE_AES_PREP;
        CRYP_CR |= CRYP_CR_CRYPEN;
        while (CRYP_SR & CRYP_SR_BUSY) __asm__("NOP");
        CRYP_CR = (CRYP_CR & ~CRYP_CR_ALGOMODE) | (CRYP_CR_ALGOMODE_AES_CBC | CRYP_CR_ALGODIR);
    } else {
        CRYP_CR = (CRYP_CR & ~CRYP_CR_ALGOMODE) | CRYP_CR_ALGOMODE_AES_CBC;
    }

    for (i=0; i<2; i++) {
        CRYP_IVR_MEW(i) = sec_info->iv[i];
    } 

    CRYP_CR |= CRYP_CR_FFLUSH;

    dma_enable_stream(DMA2, DMA_STREAM6);
    dma_enable_stream(DMA2, DMA_STREAM5);
    
    CRYP_CR |= CRYP_CR_CRYPEN;
    CRYP_DMACR = CRYP_DMACR_DIEN | CRYP_DMACR_DOEN;
    
    while ((dma2_crypt_out_complete == 0) || (dma2_crypt_in_complete == 0)) __asm__("NOP");
    
    CRYP_CR = 0;
    CRYP_DMACR = 0; 
    
    return MEW_CRYPT_OK;
}

void dma2_stream5_isr(void) {
    dma_stream_reset(DMA2, DMA_STREAM5);
    dma_disable_stream(DMA2, DMA_STREAM5);
    
    dma2_crypt_out_complete = 1;
    //debug_print("dma2_crypt_out_complete");
}

void dma2_stream6_isr(void) {
    dma_stream_reset(DMA2, DMA_STREAM6);
    dma_disable_stream(DMA2, DMA_STREAM6);
    
    dma2_crypt_in_complete = 1;
    //debug_print("dma2_crypt_in_complete");
}

u32 mewcrypt_sd_block_read(u32 block, u32* data) {
    u32 crypted[128];
    u32 state = sdio_rw512(SDIO_READ, block, crypted);
    if (state == SDIO_OK) {
        mewcrypt_aes256(MEW_DECRYPT, crypted, data, 128);
        return MEW_CRYPT_OK;
    }
    return MEW_CRYPT_ERROR;
}

u32 mewcrypt_sd_block_write(u32 block, u32* data) {
    u32 crypted[128];
    mewcrypt_aes256(MEW_ENCRYPT, data, crypted, 128);
    if (sdio_rw512(SDIO_WRITE, block, crypted) == SDIO_OK) return MEW_CRYPT_OK;
    return MEW_CRYPT_ERROR;
}

u32 mewcrypt_fram_page_read(u8 page, u32* data) {
    if (page >= FRAM_PAGES_COUNT) return MEW_CRYPT_ERROR;
    
    u32 crypted[64];
    
    
    if (i2c_fram_read_dma(page, 0, (u8*) crypted, 256) != 0) return MEW_CRYPT_ERROR;
    debug_print("mewcrypt_fram_page_read 1");
    debug_print_hex((u8*)crypted, 256);
    //if (i2c_fram_read_dma(2, 128, (u8*) (crypted+128), 128) != 0) return MEW_CRYPT_ERROR;
    
    mewcrypt_aes256(MEW_DECRYPT, crypted, data, 64);
    return MEW_CRYPT_OK;
}

u32 mewcrypt_fram_page_write(u8 page, u32* data) {
    if (page >= FRAM_PAGES_COUNT) return MEW_CRYPT_ERROR;
    
    //struct dma1_i2c1_transaction i2c_tr;
    
    u32 crypted[64];
    mewcrypt_aes256(MEW_ENCRYPT, data, crypted, 64);
    
    /*
    u8 crypted8[257];
    crypted8[0] = 0;
    memcpy(crypted8+1, crypted, 256);
    
    debug_print("mewcrypt_fram_page_write 1");
    debug_print_hex((u8*)crypted8, 257);
    
    i2c_tr.dev_addr     = (0xA0 >> 1) | (page & 0x07);
    i2c_tr.read_write   = I2C_WRITE;
    i2c_tr.buffer_count = 257;
    i2c_tr.buffer       = crypted8;
    i2c_tr.last_error   = 0;
    i2c_dma_req(&i2c_tr);
    //i2c_read_dma_wait();
    
    if (i2c_tr.last_error != 0) return MEW_CRYPT_ERROR;*/
    
    if (i2c_fram_write_dma(page, 0, (u8*) crypted, 256) != 0) return MEW_CRYPT_ERROR;
    //if (i2c_fram_write_dma(page, 128, (u8*) (crypted+128), 128) != 0) return MEW_CRYPT_ERROR;
    
    return MEW_CRYPT_OK;
}

















