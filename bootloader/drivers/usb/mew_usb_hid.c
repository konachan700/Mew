#include "mew.h"
#include "debug.h"
#include "mew_usb_hid.h"

void mew_hid_data_rx(usbd_device *usbd_dev, uint8_t ep);

usbd_device *mew_hid_usbd_dev;

uint8_t usbd_control_buffer[128];
volatile uint8_t usb_hid_disable = 1;

const unsigned char keyboard_report_descriptor[MEW_KB_REPORT_SIZE] = {
		 0x06,0x00,0xFF,                        //Usage Page 0xff00
		   0x09, 0x01,                  //USAGE (Pointer)
		   0xA1,0x01,                   //Collection (application)
		      //Input Report
		      0x19,0x01,                 //Usage Minimum
		      0x29,0x40,                 //Usage Minimum
		      0x15,0x00,                 //Logical Minimum
		      0x26,0xFF,0x00,            //Logical Minimum
		      0x75,0x08,                 //report size : 8-bit field size
		      0x95, 64,//Report count
		      0x81,0x02,                 //Input (data, array, Abs)
		      //Output Report
		      0x19,0x01,                 //usage Minimum
		      0x29,0x40,                 //usage Minimum
		      0x75,0x08,                 //report size : 8-bit field size
		      0x95,64,//Report Count
		      0x91,0x02,                 //Output (data, array, Abs)
		  0xC0
};

const struct usb_device_descriptor dev = {
	.bLength 				= USB_DT_DEVICE_SIZE,
	.bDescriptorType 		= USB_DT_DEVICE,
	.bcdUSB 				= 0x0200,
	.bDeviceClass 			= 0,
	.bDeviceSubClass 		= 0,
	.bDeviceProtocol 		= 0,
	.bMaxPacketSize0 		= 64,
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

	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_INTERRUPT, 8, &mew_hid_data_rx);
	usbd_ep_setup(usbd_dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, 8, NULL);

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
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO10 | GPIO11 | GPIO12);
	mew_hid_usbd_dev = usbd_init(&otgfs_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(mew_hid_usbd_dev, hid_set_config);
    nvic_enable_irq(NVIC_OTG_FS_IRQ);
    usb_hid_disable = 0;
    
    return 0;
}

void mew_hid_data_rx(usbd_device *usbd_dev, uint8_t ep) {
	(void)ep;
	(void)div;
	char buf[64];
	memset(buf, 0, 64);

	int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);

	mew_duart_print_hex(buf, 64);
}

void mew_hid_send(char* buf, int len) {
    if (usb_hid_disable == 1) return;
	if (len != 8) return;
	usbd_ep_write_packet(mew_hid_usbd_dev, 0x81, buf, len);
}

void otg_fs_isr(void) {
    usbd_poll(mew_hid_usbd_dev);
}

