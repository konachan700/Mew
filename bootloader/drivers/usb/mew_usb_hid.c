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
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/crc.h>

#include "drivers/system/system.h"
#include "debug.h"
#include "mew_usb_hid.h"
#include "drivers/duart/duart.h"
#include "drivers/hw_crypt/crypto.h"

void mew_hid_data_rx(usbd_device *usbd_dev, uint8_t ep);

static void mew_send_future_message(char *block);
static void mew_send_status(unsigned char status, const char *text);

usbd_device *mew_hid_usbd_dev;

uint32_t mew_hid_ring_counter = 0;
uint8_t  mew_hid_ring_buffer[64][64];

uint8_t usbd_control_buffer[128];
volatile uint8_t usb_hid_disable = 1;

static          uint8_t 	mew_usb_dfu_buffer[64];
static volatile uint8_t 	mew_usb_dfu_state 		= MEW_DFU_MODE_COMMAND;
static volatile uint32_t 	mew_usb_dfu_offset 		= 0;
static volatile uint32_t 	mew_usb_dfu_data_len	= 0;
static volatile int32_t 	mew_usb_dfu_counter		= 0;

const unsigned char keyboard_report_descriptor[MEW_KB_REPORT_SIZE] = {
	    0x06, 0x00, 0xFF,       // Usage Page = 0xFF00 (Vendor Defined Page 1)
	    0x09, 0x01,             // Usage (Vendor Usage 1)
	    0xA1, 0x01,             // Collection (Application)
	    0x19, 0x01,             //      Usage Minimum
	    0x29, 0x40,             //      Usage Maximum 	//64 input usages total (0x01 to 0x40)
	    0x15, 0x01,             //      Logical Minimum (data bytes in the report may have minimum value = 0x00)
	    0x25, 0x40,      	  	//      Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255)
	    0x75, 0x08,             //      Report Size: 8-bit field size
	    0x95, 0x40,             //      Report Count: Make sixty-four 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
	    0x81, 0x00,             //      Input (Data, Array, Abs): Instantiates input packet fields based on the above report size, count, logical min/max, and usage.
	    0x19, 0x01,             //      Usage Minimum
	    0x29, 0x40,             //      Usage Maximum 	//64 output usages total (0x01 to 0x40)
	    0x91, 0x00,             //      Output (Data, Array, Abs): Instantiates output packet fields.  Uses same report size and count as "Input" fields, since nothing new/different was specified to the parser since the "Input" item.
	    0xC0                    // End Collection
};

const struct usb_device_descriptor dev = {
	.bLength 				= USB_DT_DEVICE_SIZE,
	.bDescriptorType 		= USB_DT_DEVICE,
	.bcdUSB 				= 0x0200,
	.bDeviceClass 			= 0,
	.bDeviceSubClass 		= 0,
	.bDeviceProtocol 		= 0,
	.bMaxPacketSize0 		= 8,
	.idVendor 				= 0x1234,
	.idProduct 				= 0x4321,
	.bcdDevice 				= 0x0200,
	.iManufacturer 			= 1,
	.iProduct 				= 2,
	.iSerialNumber 			= 3,
	.bNumConfigurations     = 1,
};

const struct usb_endpoint_descriptor hid_endpoints[] = {{
	.bLength 			= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType 	= USB_DT_ENDPOINT,
	.bEndpointAddress 	= 0x81,
	.bmAttributes 		= USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize 	= 64,
	.bInterval 			= 0x01,
}, {
	.bLength 			= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType 	= USB_DT_ENDPOINT,
	.bEndpointAddress 	= 0x01,
	.bmAttributes 		= USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize 	= 64,
	.bInterval 			= 0x01,
}};

static const struct {
	struct usb_hid_descriptor hid_descriptor;
	struct {
		uint8_t bReportDescriptorType;
		uint16_t wDescriptorLength;
	} __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
	.hid_descriptor = {
		.bLength                        = sizeof(hid_function),
		.bDescriptorType 				= USB_DT_HID,
		.bcdHID 						= 0x0100,
		.bCountryCode 					= 0,
		.bNumDescriptors 				= 1,
	},
	.hid_report = {
		.bReportDescriptorType          = USB_DT_REPORT,
		.wDescriptorLength 				= sizeof(keyboard_report_descriptor),
	},
};

const struct usb_interface_descriptor hid_iface = {
	.bLength 							= USB_DT_INTERFACE_SIZE,
	.bDescriptorType 					= USB_DT_INTERFACE,
	.bInterfaceNumber 					= 0,
	.bAlternateSetting 					= 0,
	.bNumEndpoints 						= 2,
	.bInterfaceClass 					= USB_CLASS_HID,
	.bInterfaceSubClass             	= 0,
	.bInterfaceProtocol             	= 0,
	.iInterface 						= 0,
	.endpoint 							= hid_endpoints,
	.extra 								= &hid_function,
	.extralen 							= sizeof(hid_function),
};

const struct usb_interface ifaces[] = {{
	.num_altsetting 					= 1,
	.altsetting 						= &hid_iface,
}};

const struct usb_config_descriptor config = {
	.bLength 							= USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType 					= USB_DT_CONFIGURATION,
	.wTotalLength 						= 0,
	.bNumInterfaces 					= 1,
	.bConfigurationValue            	= 1,
	.iConfiguration 					= 0,
	.bmAttributes 						= 0xC0,
	.bMaxPower 							= 0x32,
	.interface 							= ifaces,
};

static const char *usb_strings[] = {
	"JNeko Lab",
	"MeW HPM Pro FWU Mode",
	"MeW00000003", // TODO: add hash of serial number
};

static int hid_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req)) {
	(void)complete;
	(void)usbd_dev;

	if ((req->bmRequestType != 0x81) || (req->bRequest != USB_REQ_GET_DESCRIPTOR) || (req->wValue != 0x2200))
		return 0;

	/* Handle the HID report descriptor. */
	*buf = (uint8_t *)keyboard_report_descriptor;
	*len = sizeof(keyboard_report_descriptor);

	return 1;
}

static void hid_set_config(usbd_device *usbd_dev, uint16_t wValue) {
	(void)wValue;
	(void)usbd_dev;

	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_INTERRUPT, 64, &mew_hid_data_rx);
	usbd_ep_setup(usbd_dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, 64, NULL);

	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				hid_control_request);
}

void mew_hid_usb_disable(void) {
    if (usb_hid_disable == 1) return;
    usb_hid_disable = 1;
    usbd_disconnect(mew_hid_usbd_dev, true);
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_clear(GPIOA, GPIO10 | GPIO11 | GPIO12);
}

unsigned int mew_hid_usb_init(void) {
	int i;
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO10 | GPIO11 | GPIO12);
	mew_hid_usbd_dev = usbd_init(&otgfs_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(mew_hid_usbd_dev, hid_set_config);
    nvic_enable_irq(NVIC_OTG_FS_IRQ);
    usb_hid_disable = 0;
    
    for (i=0; i<64; i++) mew_hid_ring_buffer[i][0] = 0;
    return 0;
}

void mew_hid_data_rx(usbd_device *usbd_dev, uint8_t ep) {
	uint32_t i = 0, offset;
	char str_buf[64], hash[32];
	char* data_p;
	(void)ep;
	(void)div;

	memset(mew_usb_dfu_buffer, 0, 64);

	int len = usbd_ep_read_packet(usbd_dev, 0x01, mew_usb_dfu_buffer, 64);
	if (mew_usb_dfu_state == MEW_DFU_MODE_COMMAND) {
		mew_usb_dfu_state 		= MEW_DFU_CMD(mew_usb_dfu_buffer);
		mew_usb_dfu_offset 		= MEW_DFU_DATA_OFFSET(mew_usb_dfu_buffer);
		mew_usb_dfu_data_len 	= MEW_DFU_DATA_LEN(mew_usb_dfu_buffer);
		if (MEW_DFU_CALC_CRC(mew_usb_dfu_buffer) != MEW_DFU_DATA_CRC(mew_usb_dfu_buffer)) {
			mew_usb_dfu_state = MEW_DFU_MODE_COMMAND;
			mew_send_status(1, "Bad checksum");
		} else {
			switch (mew_usb_dfu_state) {
			case MEW_DFU_MODE_WRITE_FW:
				mew_usb_dfu_counter = 0;
				flash_unlock();
				for (i=4; i<=11; i++) {
					flash_erase_sector(i, 0);
				}
				flash_lock();
				mew_send_status(13, "Erase OK. Program mode enable.");
				return;
			case MEW_DFU_MODE_VERIFY:
				mew_send_status(12, "Flash verify mode enable.");
				break;
			}
		}
		return;
	} else {
		switch (mew_usb_dfu_state) {
		case MEW_DFU_MODE_WRITE_FW:
			offset = APP_ADDRESS + mew_usb_dfu_counter + mew_usb_dfu_offset;

			flash_unlock();
			flash_program(offset, mew_usb_dfu_buffer, 64);
			flash_lock();

			mew_usb_dfu_counter = mew_usb_dfu_counter + 64;
			if (mew_usb_dfu_counter > mew_usb_dfu_data_len) {
				mew_send_status(11, "OK                ");
				mew_usb_dfu_counter = 0;
				mew_usb_dfu_state = MEW_DFU_MODE_COMMAND;
			} else {
				sprintf(str_buf, "0x%08lX", mew_usb_dfu_counter);
				mew_send_status(0xFF, str_buf);
			}
			break;
		case MEW_DFU_MODE_VERIFY:
			offset = APP_ADDRESS + mew_usb_dfu_offset;
			data_p = (char *) offset;
			mew_sha256_init();
			for (i=0; i<mew_usb_dfu_data_len; i++) {
				mew_sha256_add_byte(data_p[i]);
			}
			mew_sha256_finalize((uint8_t*) hash);

			if (memcmp(mew_usb_dfu_buffer, hash, 32) == 0) {
				mew_send_status(11, "Verify OK.");
			} else {
				mew_send_status(11, "Verify failed.");
			}

			memset(mew_usb_dfu_buffer, 0, 64);
			memcpy(mew_usb_dfu_buffer, hash, 32);
			mew_send_status(27, (const char *) mew_usb_dfu_buffer);
			mew_usb_dfu_state = MEW_DFU_MODE_COMMAND;
			break;
		}
	}
}

static void mew_send_future_message(char *block) {
	memcpy(mew_hid_ring_buffer[mew_hid_ring_counter], block, 64);
	mew_hid_ring_counter++;
	if (mew_hid_ring_counter >= 64) {
		mew_hid_ring_counter = 0;
	}
}

void mew_print_loop_handler(void) {
	int i;
	for (i=0; i<64; i++) {
		if (mew_hid_ring_buffer[i][0] > 0) {
			while (usbd_ep_write_packet(mew_hid_usbd_dev, 0x81, mew_hid_ring_buffer[i], 64) == 0) {};
			mew_delay_ms(1);
			mew_hid_ring_buffer[i][0] = 0;
		}
	}
}

static void mew_send_status(unsigned char status, const char *text) {
	int len;
	char data[64];
	memset(data, 0, 64);

	data[0] = (char) status;
	if (text != NULL) {
		len = strlen(text);
		if (len > 60) len = 60;
		memcpy(data + 1, text, len);
	}
	mew_send_future_message(data);
}

void mew_hid_send(char* buf, int len) {
    if (usb_hid_disable == 1) return;
	usbd_ep_write_packet(mew_hid_usbd_dev, 0x81, buf, len);
}

void otg_fs_isr(void) {
    usbd_poll(mew_hid_usbd_dev);
}

