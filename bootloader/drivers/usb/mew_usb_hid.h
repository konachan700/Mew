#ifndef __USB_MEW__
#define __USB_MEW__

#include "mew.h"

#include <stdlib.h>
#include <string.h>

#include <libopencm3/stm32/f4/nvic.h>
#include <libopencm3/stm32/f4/memorymap.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/cm3/nvic.h>

#define MEW_DFU_MODE_COMMAND					0x00
#define MEW_DFU_MODE_WRITE_FW					0x43
#define MEW_DFU_MODE_VERIFY						0x71

#define MEW_DFU_CMD(a) 							a[0]
#define MEW_DFU_DATA_LEN(a)						((a[1] << 16) | (a[2] << 8) | a[3])
#define MEW_DFU_DATA_OFFSET(a)					((a[4] << 16) | (a[5] << 8) | a[6])
#define MEW_DFU_DATA_CRC(a)						a[7]

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05 

#define HID_DESCRIPTOR_TYPE                     0x21

#define MEW_KB_REPORT_SIZE                      28
#define MEW_CUSTOM_HID_REPORT_SIZE              64

#define MEW_UPPER_CASE                          0x02
#define MEW_LOWER_CASE                          0x00

unsigned int mew_hid_usb_init(void);

void mew_hid_usb_disable(void);
void mew_hid_send(char* buf, int len);
void mew_hid_send_char(char ch, char char_case);

#endif
