#include "sdcard.h" 

volatile u8 dma2_sdio_complete = 1;

volatile u32 sdio_status_ccs = 0;
volatile u32 sdio_status_rca = 0;
volatile u32 sdio_card_size  = 0;

u32 conf_disk_temporary_sector[128];

static const struct usb_device_descriptor dev_descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0110,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0483,
	.idProduct = 0x5741,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

static const struct usb_endpoint_descriptor msc_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 0,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 0,
} };

static const struct usb_interface_descriptor msc_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_MSC,
	.bInterfaceSubClass = USB_MSC_SUBCLASS_SCSI,
	.bInterfaceProtocol = USB_MSC_PROTOCOL_BBB,
	.iInterface = 0,
	.endpoint = msc_endp,
	.extra = NULL,
	.extralen = 0
} };

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = msc_iface,
} };

static const struct usb_config_descriptor config_descr = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,
	.interface = ifaces,
};

static const char *usb_strings[] = {
	"JNeko MeW HPM config device",
	"MeW Config flash drive",
	"MeW-conf",
};

usbd_device *config_msc_dev;
static uint8_t usbd_control_buffer[128];

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
    //debug_print("SDIO OK!");
    //debug_print_hex((u8*) &sdio_card_size, 4);
}

u32 __sdio_wait_dr(void) {
    u32 timer = 0;
    while (timer < SDIO_CMD41_TIMER) {
        timer++;
        if ((__sdio_cmd_wait(13, sdio_status_rca << 16) == SDIO_ESUCCESS) && ((SDIO_RESP1 & 0x100) != 0)) return SDIO_OK;
    }
    return SDIO_GENERAL_ERROR;
}

u32 __sdio_get_r1(void) {
    if (__sdio_cmd_wait(13, sdio_status_rca << 16) == SDIO_ESUCCESS) return (SDIO_RESP1 >> 9) & 0x0F;
    return SDIO_R1_ERROR;
}

u32 sdio_rw512(u8 rw, u32 address, u32 *buffer) {
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
    
    nvic_enable_irq(NVIC_DMA2_STREAM3_IRQ);
    dma_stream_reset(DMA2, DMA_STREAM3);
    
    if (rw == SDIO_READ)
        dma_set_transfer_mode(DMA2, DMA_STREAM3, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
    else 
        dma_set_transfer_mode(DMA2, DMA_STREAM3, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    
    dma_set_priority(DMA2,  DMA_STREAM3, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(DMA2, DMA_STREAM3, DMA_SxCR_MSIZE_32BIT);
    dma_set_peripheral_size(DMA2, DMA_STREAM3, DMA_SxCR_PSIZE_32BIT);
    dma_enable_memory_increment_mode(DMA2, DMA_STREAM3);
    dma_set_peripheral_address(DMA2, DMA_STREAM3, (u32) &SDIO_FIFO);
    dma_channel_select(DMA2, DMA_STREAM3, DMA_SxCR_CHSEL_4);
    dma_set_memory_address(DMA2, DMA_STREAM3, (u32) buffer);
    dma_set_number_of_data(DMA2, DMA_STREAM3, 128);
    
    dma_enable_fifo_mode(DMA2, DMA_STREAM3);
    dma_set_peripheral_flow_control(DMA2, DMA_STREAM3);
    dma_set_fifo_threshold(DMA2, DMA_STREAM3, DMA_SxFCR_FTH_4_4_FULL);
    dma_set_memory_burst(DMA2, DMA_STREAM3, DMA_SxCR_MBURST_INCR4);
    dma_set_peripheral_burst(DMA2, DMA_STREAM3, DMA_SxCR_PBURST_INCR4); 
    
    dma_disable_fifo_error_interrupt(DMA2, DMA_STREAM3);
    dma_disable_half_transfer_interrupt(DMA2, DMA_STREAM3);
    dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM3);
    dma_enable_transfer_error_interrupt(DMA2,    DMA_STREAM3);
    dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM3);
    dma_enable_stream(DMA2, DMA_STREAM3);

    if (rw == SDIO_READ) {
        SDIO_DTIMER = 4800000;
        SDIO_DLEN = 512;
        SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_9 | SDIO_DCTRL_DMAEN | SDIO_DCTRL_DTDIR | SDIO_DCTRL_DTEN;
        
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
    } else {
        if (__sdio_cmd_wait(24, address) != SDIO_ESUCCESS) {
            debug_print("SDIO ERR RD: CMD 24");
            return SDIO_GENERAL_ERROR;
        }        
        
        SDIO_DTIMER = 0xFFFFFFFF;
        SDIO_DLEN = 512;
        SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_9 | SDIO_DCTRL_DMAEN | SDIO_DCTRL_DTEN;
                
        while (true) {
            u32 result = SDIO_STA;
            if (result & (DATA_TX_SUCCESS_FLAGS | DATA_TX_ERROR_FLAGS)) {
                if (result & DATA_TX_ERROR_FLAGS) {
                    debug_print("SDIO ERR WR: DATA_TX_ERROR_FLAGS");
                    //debug_print_hex((u8*) &result, 4);
                    return SDIO_GENERAL_ERROR;
                }
                break;
            }
        } 
    }
    
    while (__sdio_get_r1() !=  SDIO_R1_TRAN) __asm__("NOP");
    while (dma2_sdio_complete == 0) __asm__("NOP");
    return SDIO_OK;
}

void dma2_stream3_isr(void) {
    dma_stream_reset(DMA2, DMA_STREAM3);
    dma_disable_stream(DMA2, DMA_STREAM3);
    
    dma2_sdio_complete = 1;
    //debug_print("dma2_stream3_isr");
}

int __sd_config_disk_read(uint32_t lba, uint8_t *copy_to) {
    //debug_print("__sd_config_disk_read() reading...");
    if (sdio_rw512(SDIO_READ, lba + MAX_PASSWORDS_COUNT, conf_disk_temporary_sector) != SDIO_OK) {
        debug_print("__sd_config_disk_read() error!");
        return 1;
    }
    memcpy(copy_to, conf_disk_temporary_sector, 512);
    return 0;
}

int __sd_config_disk_write(uint32_t lba, const uint8_t *copy_from) {
    //debug_print("__sd_config_disk_write() writing...");
    memcpy(conf_disk_temporary_sector, copy_from, 512);
    if (sdio_rw512(SDIO_WRITE, lba + MAX_PASSWORDS_COUNT, conf_disk_temporary_sector) != SDIO_OK) {
        debug_print("__sd_config_disk_write() error!");
        return 1;
    }
    return 0;
}

void start_usb_msd_config(void) {    
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO10 | GPIO11 | GPIO12);
    
    config_msc_dev = usbd_init(&otgfs_usb_driver, &dev_descr, &config_descr, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
    usb_msc_init(config_msc_dev, 0x82, 64, 0x01, 64, "JNeko", "MeW HPM","0.00", CONFIG_DISK_SIZE, __sd_config_disk_read, __sd_config_disk_write);
    
    //nvic_enable_irq(NVIC_OTG_FS_IRQ);
}

