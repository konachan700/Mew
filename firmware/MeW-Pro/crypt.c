#include "crypt.h"  

struct security_information* sec_info;

volatile u8 dma2_crypt_in_complete = 1;
volatile u8 dma2_crypt_out_complete = 1;

u32 temporary_sector[128];

static inline struct security_information* crypt_get_security_information(void) {
	return (struct security_information*)(FLASH_SI_GLOBAL_OFFSET);
}

u32 mewcrypt_aes256_gen_keys(void) {
    sec_info = crypt_get_security_information();
    if (sec_info->magic != MEW_SECURITY_INFO_MAGIC) {
        struct security_information sec_info_temp;
        sec_info_temp.magic = MEW_SECURITY_INFO_MAGIC;
        sec_info_temp.uid = random_u32();
        memset_random_u32((u32*) sec_info_temp.key, 8);
        memset_random_u32((u32*) sec_info_temp.iv, 4);
        
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
    u32 state = sdio_rw512(SDIO_READ, block, temporary_sector);
    if (state == SDIO_OK) {
        mewcrypt_aes256(MEW_DECRYPT, temporary_sector, data, 128);
        return MEW_CRYPT_OK;
    }
    return MEW_CRYPT_ERROR;
}

u32 mewcrypt_sd_block_write(u32 block, u32* data) {
    mewcrypt_aes256(MEW_ENCRYPT, data, temporary_sector, 128);
    if (sdio_rw512(SDIO_WRITE, block, temporary_sector) == SDIO_OK) return MEW_CRYPT_OK;
    return MEW_CRYPT_ERROR;
}

u32 mewcrypt_fram_page_read(u8 page, u32* data) {
    if (page >= FRAM_PAGES_COUNT) return MEW_CRYPT_ERROR;
    if (i2c_fram_read_dma(page, 0, (u8*) temporary_sector, 256) != 0) return MEW_CRYPT_ERROR;
    mewcrypt_aes256(MEW_DECRYPT, temporary_sector, data, 64);
    return MEW_CRYPT_OK;
}

u32 mewcrypt_fram_page_write(u8 page, u32* data) {
    if (page >= FRAM_PAGES_COUNT) return MEW_CRYPT_ERROR;
    mewcrypt_aes256(MEW_ENCRYPT, data, temporary_sector, 64);
    if (i2c_fram_write_dma(page, 0, (u8*) temporary_sector, 256) != 0) return MEW_CRYPT_ERROR;
    return MEW_CRYPT_OK;
}

u32 __mewcrypt_generate_root_password_dir(struct password_sector* ps) {   
    if (ps->crc32 != crc_gen((u32*) (&ps->password), sizeof(struct password_record) / sizeof(u32))) {
        struct password_record ps_new;
        ps_new.magic = MEW_PASSWORD_RECORD_MAGIC;
        ps_new.id = 0;
        ps_new.parent_id = MEW_PASSWORD_RECORD_NO_PARENT;
        ps_new.display_number = 0;
        memset(ps_new.title, 0xFF, MEW_PASSWORD_RECORD_TITLE_LEN);
        memset(ps_new.text, 0xFF, MEW_PASSWORD_RECORD_TEXT_LEN);
        memset(ps_new.login, 0xFF, MEW_PASSWORD_RECORD_TEXT_LEN);
        ps_new.icon = icon_E0DA;
        ps_new.flags = 0;
        memset(ps_new.extra, 0x00, 256);
        
        #ifdef CREATE_DEMO_PASSWORDS
        ps_new.extra[0] = 0x01;
        ps_new.extra[1] = 0x02;
        ps_new.extra[2] = 0x03;        
        #endif
        
        memset(temporary_sector, 0x00, 512);
        
        ps->crc32 = crc_gen((u32*) &ps_new, sizeof(struct password_record) / sizeof(u32));
        memcpy(&ps->password, &ps_new, sizeof(struct password_record));

        if (mewcrypt_sd_block_write(0, temporary_sector) == MEW_CRYPT_ERROR) 
            return MEW_CRYPT_ERROR;
        
        #ifdef CREATE_DEMO_PASSWORDS
        
        ps_new.id = 1;
        ps_new.parent_id = 0;
        strncpy(ps_new.title, "DEMO 01", MEW_PASSWORD_RECORD_TITLE_LEN);
        strncpy(ps_new.text, "Test password MeW 01", MEW_PASSWORD_RECORD_TEXT_LEN);
        strncpy(ps_new.login, "MeWLoginX1", MEW_PASSWORD_RECORD_TEXT_LEN);
        ps_new.icon = NULL;
        ps_new.flags = PASSWORD_FLAG_DIRECTORY;
        ps_new.extra[0] = 0x04;
        ps_new.extra[1] = 0x05;
        ps_new.extra[2] = 0x00;
        mewcrypt_write_pr(&ps_new, ps_new.id);

        ps_new.id = 2;
        ps_new.parent_id = 0;
        strncpy(ps_new.title, "DEMO 02", MEW_PASSWORD_RECORD_TITLE_LEN);
        strncpy(ps_new.text, "Test password MeW 02", MEW_PASSWORD_RECORD_TEXT_LEN);
        strncpy(ps_new.login, "MeWLoginX1", MEW_PASSWORD_RECORD_TEXT_LEN);
        ps_new.icon = NULL;
        ps_new.flags = PASSWORD_FLAG_DIRECTORY;
        ps_new.extra[0] = 0x00;
        ps_new.extra[1] = 0x00;
        ps_new.extra[2] = 0x00;
        mewcrypt_write_pr(&ps_new, ps_new.id);
        
        ps_new.id = 3;
        ps_new.parent_id = 0;
        strncpy(ps_new.title, "DEMO 03", MEW_PASSWORD_RECORD_TITLE_LEN);
        strncpy(ps_new.text, "Test password MeW 03", MEW_PASSWORD_RECORD_TEXT_LEN);
        strncpy(ps_new.login, "MeWLoginX1", MEW_PASSWORD_RECORD_TEXT_LEN);
        ps_new.icon = NULL;
        ps_new.flags = 0;
        mewcrypt_write_pr(&ps_new, ps_new.id);
        
        ps_new.id = 4;
        ps_new.parent_id = 1;
        strncpy(ps_new.title, "DEMO 04", MEW_PASSWORD_RECORD_TITLE_LEN);
        strncpy(ps_new.text, "Test password MeW 04", MEW_PASSWORD_RECORD_TEXT_LEN);
        strncpy(ps_new.login, "MeWLoginX1", MEW_PASSWORD_RECORD_TEXT_LEN);
        ps_new.icon = NULL;
        ps_new.flags = PASSWORD_FLAG_DIRECTORY;
        ps_new.extra[0] = 0x06;
        ps_new.extra[1] = 0x07;
        ps_new.extra[2] = 0x00;
        mewcrypt_write_pr(&ps_new, ps_new.id);
        
        ps_new.id = 5;
        ps_new.parent_id = 1;
        strncpy(ps_new.title, "DEMO 05", MEW_PASSWORD_RECORD_TITLE_LEN);
        strncpy(ps_new.text, "Test password MeW 05", MEW_PASSWORD_RECORD_TEXT_LEN);
        strncpy(ps_new.login, "MeWLoginX1", MEW_PASSWORD_RECORD_TEXT_LEN);
        ps_new.icon = NULL;
        ps_new.flags = 0;
        ps_new.extra[0] = 0x00;
        ps_new.extra[1] = 0x00;
        ps_new.extra[2] = 0x00;
        mewcrypt_write_pr(&ps_new, ps_new.id);
        
        ps_new.id = 6;
        ps_new.parent_id = 4;
        strncpy(ps_new.title, "DEMO 06", MEW_PASSWORD_RECORD_TITLE_LEN);
        strncpy(ps_new.text, "Test password MeW 06", MEW_PASSWORD_RECORD_TEXT_LEN);
        strncpy(ps_new.login, "MeWLoginX1", MEW_PASSWORD_RECORD_TEXT_LEN);
        ps_new.icon = NULL;
        ps_new.flags = 0;
        mewcrypt_write_pr(&ps_new, ps_new.id);
        
        ps_new.id = 7;
        ps_new.parent_id = 4;
        strncpy(ps_new.title, "DEMO 07", MEW_PASSWORD_RECORD_TITLE_LEN);
        strncpy(ps_new.text, "Test password MeW 07", MEW_PASSWORD_RECORD_TEXT_LEN);
        strncpy(ps_new.login, "MeWLoginX1", MEW_PASSWORD_RECORD_TEXT_LEN);
        ps_new.icon = NULL;
        ps_new.flags = 0;
        mewcrypt_write_pr(&ps_new, ps_new.id);
        
        #endif

        debug_print("Root record corrupted or not exist!");
        debug_print("New root record was created.");
        
        return MEW_CRYPT_ERROR;
    }
    return MEW_CRYPT_OK;
}

u32 mewcrypt_get_pwd_record(struct password_record* pr, u32 index) {
    struct password_sector* ps = (struct password_sector*) temporary_sector;
    
    if (mewcrypt_sd_block_read(index, temporary_sector) == MEW_CRYPT_ERROR) 
        return MEW_CRYPT_ERROR;
    
    if (index == 0) {
        if (__mewcrypt_generate_root_password_dir(ps) != MEW_CRYPT_OK) {
            if (mewcrypt_sd_block_read(index, temporary_sector) == MEW_CRYPT_ERROR) 
                return MEW_CRYPT_ERROR;
        }
    }
    
    if (ps->crc32 != crc_gen((u32*) (&ps->password), sizeof(struct password_record) / sizeof(u32))) 
        return MEW_CRYPT_ERROR;
    
    memcpy(pr, &ps->password, sizeof(struct password_record));
    
    return MEW_CRYPT_OK;
}

u32 mewcrypt_write_pr(struct password_record* pr, u32 index) {
    //if (index == 0) return MEW_CRYPT_ERROR;
    memset(temporary_sector, 0x00, 512);
    
    struct password_sector* ps = (struct password_sector*) temporary_sector;
    ps->crc32 = crc_gen((u32*) (pr), sizeof(struct password_record) / sizeof(u32));
    memcpy(&ps->password, pr, sizeof(struct password_record));
    
    if (mewcrypt_sd_block_write(index, temporary_sector) == MEW_CRYPT_ERROR) 
        return MEW_CRYPT_ERROR;

    return MEW_CRYPT_OK;
}

u32 mewcrypt_read_settings(struct settings_record* sr, u8 index) {
    memset(temporary_sector, 0x00, 512);
    
    if (mewcrypt_fram_page_read(index, temporary_sector) == MEW_CRYPT_ERROR)
        return MEW_CRYPT_ERROR;
        
    struct settings_eeprom_sector* sc = (struct settings_eeprom_sector*) temporary_sector;
    if (sc->crc32 != crc_gen((u32*) (&sc->settings), sizeof(struct settings_record) / sizeof(u32))) {
        memset(temporary_sector, 0x00, 512);
        
        struct settings_record tmp;
        memset(&tmp, 0x00, sizeof(struct settings_record));
        tmp.magic = MEW_SETTING_MAGIC;
        tmp.global_mode = MEW_GLOBAL_MODE_HID;
        mewcrypt_write_settings(&tmp, index);
        
        if (mewcrypt_fram_page_read(index, temporary_sector) == MEW_CRYPT_ERROR)
            return MEW_CRYPT_ERROR;
        
        if (sc->crc32 != crc_gen((u32*) (&sc->settings), sizeof(struct settings_record) / sizeof(u32)))
            MEW_CRYPT_ERROR;
        
        debug_print("Eeprom settings record corrupted or not exist!");
        debug_print("New settings record was created.");
    }
    
    memcpy(sr, &sc->settings, sizeof(struct settings_record));
    
    return MEW_CRYPT_OK;
}

u32 mewcrypt_write_settings(struct settings_record* sr, u8 index) {
    memset(temporary_sector, 0x00, 512);
    struct settings_eeprom_sector* sc = (struct settings_eeprom_sector*) temporary_sector;
    memcpy(&sc->settings, sr, sizeof(struct settings_record));
    
    sc->crc32 = crc_gen((u32*) sr, sizeof(struct settings_record) / sizeof(u32));
    if (mewcrypt_fram_page_write(index, temporary_sector) == MEW_CRYPT_ERROR)
        return MEW_CRYPT_ERROR;
    
    
    
    return MEW_CRYPT_OK;
}













































