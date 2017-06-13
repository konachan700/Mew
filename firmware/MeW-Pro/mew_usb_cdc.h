#ifndef _USB_CDC_MEW_
#define _USB_CDC_MEW_

#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h> 

#include "board.h" 
#include "config_mode.h"

extern void cdc_acm_start(void);
extern void cdcacm_send_chars(u8* data, u16 len);

#endif
