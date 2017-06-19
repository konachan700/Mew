#include "mew_usb_hid.h"

usbd_device *mew_hid_usbd_dev;

uint8_t usbd_control_buffer[128];
volatile u8 usb_hid_disable = 1;

const unsigned char keyboard_report_descriptor[MEW_KB_REPORT_SIZE] =
{
	0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	0x09, 0x06, // USAGE (Keyboard)
	0xa1, 0x01, // COLLECTION (Application)
	0x05, 0x07, //     USAGE_PAGE (Keyboard/Keypad)
	0x19, 0xe0, //     USAGE_MINIMUM (Keyboard LeftControl)
	0x29, 0xe7, //     USAGE_MAXIMUM (Keyboard Right GUI)
	0x15, 0x00, //     LOGICAL_MINIMUM (0)
	0x25, 0x01, //     LOGICAL_MAXIMUM (1)
	0x95, 0x08, //     REPORT_COUNT (8)
	0x75, 0x01, //     REPORT_SIZE (1)
	0x81, 0x02, //     INPUT (Data, Var, Abs)
	0x95, 0x01, //     REPORT_COUNT (1)
	0x75, 0x08, //     REPORT_SIZE (8)
	0x81, 0x03, //     INPUT (Cnst, Var, Abs)
	0x95, 0x06, //   REPORT_COUNT (6)
	0x75, 0x08, //   REPORT_SIZE (8)
	0x25, 0xFF, //   LOGICAL_MAXIMUM (255)
	0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))
	0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)
	0x81, 0x00, //     INPUT (Data, Ary, Abs)
	0x25, 0x01, //     LOGICAL_MAXIMUM (1)
	0x95, 0x02, //   REPORT_COUNT (2)
	0x75, 0x01, //   REPORT_SIZE (1)
	0x05, 0x08, //   USAGE_PAGE (LEDs)
	0x19, 0x01, //   USAGE_MINIMUM (Num Lock)
	0x29, 0x02, //   USAGE_MAXIMUM (Caps Lock)
	0x91, 0x02, //   OUTPUT (Data, Var, Abs)
	0x95, 0x01, //   REPORT_COUNT (1)
	0x75, 0x06, //   REPORT_SIZE (6)
	0x91, 0x03, //   OUTPUT (Cnst, Var, Abs)
	0xc0        // END_COLLECTION
};

const struct usb_device_descriptor dev = {
	.bLength 		= USB_DT_DEVICE_SIZE,
	.bDescriptorType 	= USB_DT_DEVICE,
	.bcdUSB 		= 0x0200,
	.bDeviceClass 		= 0,
	.bDeviceSubClass 	= 0,
	.bDeviceProtocol 	= 0,
	.bMaxPacketSize0 	= 64,
	.idVendor 		= 0x1234,
	.idProduct 		= 0x4321,
	.bcdDevice 		= 0x0200,
	.iManufacturer 		= 1,
	.iProduct 		= 2,
	.iSerialNumber 		= 3,
	.bNumConfigurations     = 1,
};

const struct usb_endpoint_descriptor hid_endpoints[] = {{
	.bLength 		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType 	= USB_DT_ENDPOINT,
	.bEndpointAddress 	= 0x81,
	.bmAttributes 		= USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize 	= 8,
	.bInterval 		= 0x04,
}, {
	.bLength 		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType 	= USB_DT_ENDPOINT,
	.bEndpointAddress 	= 0x01,
	.bmAttributes 		= USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize 	= 8,
	.bInterval 		= 0x04,
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
		.bDescriptorType 		= USB_DT_HID,
		.bcdHID 			= 0x0100,
		.bCountryCode 			= 0,
		.bNumDescriptors 		= 1,
	},
	.hid_report = {
		.bReportDescriptorType          = USB_DT_REPORT,
		.wDescriptorLength 		= sizeof(keyboard_report_descriptor),
	},
};

const struct usb_interface_descriptor hid_iface = {
	.bLength 				= USB_DT_INTERFACE_SIZE,
	.bDescriptorType 		= USB_DT_INTERFACE,
	.bInterfaceNumber 		= 0,
	.bAlternateSetting 		= 0,
	.bNumEndpoints 			= 2,
	.bInterfaceClass 		= USB_CLASS_HID,
	.bInterfaceSubClass             = 1,
	.bInterfaceProtocol             = 1,
	.iInterface 			= 0,
	.endpoint 			= hid_endpoints,
	.extra 				= &hid_function,
	.extralen 			= sizeof(hid_function),
};

const struct usb_interface ifaces[] = {{
	.num_altsetting 		= 1,
	.altsetting 			= &hid_iface,
}};

const struct usb_config_descriptor config = {
	.bLength 			= USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType 		= USB_DT_CONFIGURATION,
	.wTotalLength 			= 0,
	.bNumInterfaces 		= 1,
	.bConfigurationValue            = 1,
	.iConfiguration 		= 0,
	.bmAttributes 			= 0xC0,
	.bMaxPower 			= 0x32,
	.interface 			= ifaces,
};

static const char *usb_strings[] = {
	"JNeko Lab",
	"MeW HPM Pro",
	"MeW00000001",
};

static int hid_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
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

	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_INTERRUPT, 8, NULL);
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

void mew_hid_usb_init(void) {
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO10 | GPIO11 | GPIO12);
	mew_hid_usbd_dev = usbd_init(&otgfs_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(mew_hid_usbd_dev, hid_set_config);
    nvic_enable_irq(NVIC_OTG_FS_IRQ);
    usb_hid_disable = 0;
}

void mew_hid_send(char* buf, int len) {
    if (usb_hid_disable == 1) return;
	if (len != 8) return;
	usbd_ep_write_packet(mew_hid_usbd_dev, 0x81, buf, len);
}

void mew_hid_send_char(char ch, char char_case) {
    if (usb_hid_disable == 1) return;
	unsigned const char null_buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned const char buf[8] = { char_case, 0, ch, 0, 0, 0, 0, 0 };
	if ((ch <= 0x03) || (ch >= 0x29)) return;
	while (usbd_ep_write_packet(mew_hid_usbd_dev, 0x81, buf, 8) == 0);
	while (usbd_ep_write_packet(mew_hid_usbd_dev, 0x81, null_buf, 8) == 0);
}
