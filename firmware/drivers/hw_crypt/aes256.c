#include "debug.h"
#include "crypto.h"
#include "../system/system.h"

#define CRYP_IVR_MEW(i) MMIO64(CRYP_BASE + 0x40 + (i) * 8)

static volatile uint8_t dma2_crypt_in_complete = 1;
static volatile uint8_t dma2_crypt_out_complete = 1;

void mewcrypt_aes256(uint8_t dir, uint32_t* in_block, uint32_t* out_block, uint32_t count, const uint64_t* key, const uint64_t* iv) {
    uint8_t i;
    
    mew_check_dma_memory((void*)in_block, "mewcrypt_aes256 in_block");
    mew_check_dma_memory((void*)out_block, "mewcrypt_aes256 out_block");
    
    dma2_crypt_in_complete = 0;
    nvic_enable_irq(NVIC_DMA2_STREAM6_IRQ);
    
    dma_disable_stream(DMA2, DMA_STREAM6);
    dma_stream_reset(DMA2, DMA_STREAM6);
    dma_set_transfer_mode(DMA2, DMA_STREAM6, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(DMA2,  DMA_STREAM6, DMA_SxCR_PL_LOW);
    dma_set_memory_size(DMA2, DMA_STREAM6, DMA_SxCR_MSIZE_32BIT);
    dma_set_peripheral_size(DMA2, DMA_STREAM6, DMA_SxCR_PSIZE_32BIT);
    dma_enable_memory_increment_mode(DMA2, DMA_STREAM6);
    dma_set_peripheral_address(DMA2, DMA_STREAM6, (uint32_t) &CRYP_DIN);
    dma_channel_select(DMA2, DMA_STREAM6, DMA_SxCR_CHSEL_2);
    dma_set_memory_address(DMA2, DMA_STREAM6, (uint32_t) in_block);
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
    dma_set_peripheral_address(DMA2, DMA_STREAM5, (uint32_t) &CRYP_DOUT);
    dma_channel_select(DMA2, DMA_STREAM5, DMA_SxCR_CHSEL_2);
    dma_set_memory_address(DMA2, DMA_STREAM5, (uint32_t) out_block);
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
        CRYP_KR(i) = key[i];
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
        CRYP_IVR_MEW(i) = iv[i];
    } 

    CRYP_CR |= CRYP_CR_FFLUSH;
    CRYP_CR |= CRYP_CR_CRYPEN;
    CRYP_DMACR = CRYP_DMACR_DIEN | CRYP_DMACR_DOEN;
    
    dma_enable_stream(DMA2, DMA_STREAM5);
    dma_enable_stream(DMA2, DMA_STREAM6);
    
    while ((dma2_crypt_out_complete == 0) || (dma2_crypt_in_complete == 0)) __asm__("NOP");
    
    CRYP_CR = 0;
    CRYP_DMACR = 0; 
}

void dma2_stream5_isr(void) {
    dma_stream_reset(DMA2, DMA_STREAM5);
    dma_disable_stream(DMA2, DMA_STREAM5);
    
    dma2_crypt_out_complete = 1;
    //mew_debug_print("dma2_crypt_out_complete");
}

void dma2_stream6_isr(void) {
    dma_stream_reset(DMA2, DMA_STREAM6);
    dma_disable_stream(DMA2, DMA_STREAM6);
    
    dma2_crypt_in_complete = 1;
    //mew_debug_print("dma2_crypt_in_complete");
}
