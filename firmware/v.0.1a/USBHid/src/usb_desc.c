/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : usb_desc.c
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Descriptors for Joystick Mouse Demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* USB Standard Device Descriptor */
const u8 Joystick_DeviceDescriptor[JOYSTICK_SIZ_DEVICE_DESC] =
  {
    0x12,                       /*bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType*/
    0x00,                       /*bcdUSB */
    0x02,
    0x00,                       /*bDeviceClass*/
    0x00,                       /*bDeviceSubClass*/
    0x00,                       /*bDeviceProtocol*/
    0x40,                       /*bMaxPacketSize40*/
    0x34,                       /*idVendor (0x1234)*/
    0x12,
    0x21,                       /*idProduct = 0x4321*/
    0x43,
    0x00,                       /*bcdDevice rel. 2.00*/
    0x02,
    1,                          /*Index of string descriptor describing
                                              manufacturer */
    2,                          /*Index of string descriptor describing
                                             product*/
    3,                          /*Index of string descriptor describing the
                                             device serial number */
    0x01                        /*bNumConfigurations*/
  }
  ; /* Joystick_DeviceDescriptor */


/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
const u8 Joystick_ConfigDescriptor[JOYSTICK_SIZ_CONFIG_DESC] =
  {
 //以下为配置描述符
 0x09, /* bLength: Configuation Descriptor size */
 USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
 JOYSTICK_SIZ_CONFIG_DESC,
 /* wTotalLength: Bytes returned */
 0x00,
 0x01,         /*bNumInterfaces: 1 interface*/
 0x01,         /*bConfigurationValue: Configuration value*/
 0x00,         /*iConfiguration: Index of string descriptor describing
               the configuration*/
 0xC0,         /*bmAttributes: self powered */
 0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

 //以下为接口描述符
 /************** Descriptor of Joystick Mouse interface ****************/
 /* 09 */
 0x09,         /*bLength: Interface Descriptor size*/
 USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
 0x00,         /*bInterfaceNumber: Number of Interface*/
 0x00,         /*bAlternateSetting: Alternate setting*/
 0x02,         /*bNumEndpoints*/
 0x03,         /*bInterfaceClass: HID*/
 0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
 0x01,         /*bInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
 0,            /*iInterface: Index of string descriptor*/

 //以下为HID描述符
 /******************** Descriptor of Joystick Mouse HID ********************/
 /* 18 */
 0x09,         /*bLength: HID Descriptor size*/
 HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
 0x00,         /*bcdHID: HID Class Spec release number*/
 0x01,
 0x00,         /*bCountryCode: Hardware target country*/
 0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
 0x22,         /*bDescriptorType*/
 JOYSTICK_SIZ_REPORT_DESC,/*wItemLength: Total length of Report descriptor*/
 0x00,
 
 //以下为输入端点1描述符
 /******************** Descriptor of Joystick Mouse endpoint ********************/
 /* 27 */
 0x07,          /*bLength: Endpoint Descriptor size*/
 USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
 0x81,          /*bEndpointAddress: Endpoint Address (IN)*/
 0x03,          /*bmAttributes: Interrupt endpoint*/
 0x08,          /*wMaxPacketSize: 8 Byte max */
 0x00,
 0x08,          /*bInterval: Polling Interval (8 ms)*/

 //以下为输出端点1描述符
 /* 34 */
 0x07,          /*bLength: Endpoint Descriptor size*/
 USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
 0x01,          /*bEndpointAddress: Endpoint Address (OUT)*/
 0x03,          /*bmAttributes: Interrupt endpoint*/
 0x08,          /*wMaxPacketSize: 8 Byte max */
 0x00,
 0x08,          /*bInterval: Polling Interval (8 ms)*/
/* 41 */
}; /* MOUSE_ConfigDescriptor */


const u8 Joystick_ReportDescriptor[JOYSTICK_SIZ_REPORT_DESC] =
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
 0x81, 0x02, //     INPUT (Data,Var,Abs)
 0x95, 0x01, //     REPORT_COUNT (1)
 0x75, 0x08, //     REPORT_SIZE (8)
 0x81, 0x03, //     INPUT (Cnst,Var,Abs)
 0x95, 0x06, //   REPORT_COUNT (6)
 0x75, 0x08, //   REPORT_SIZE (8)
 0x25, 0xFF, //   LOGICAL_MAXIMUM (255)
 0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))
 0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)
 0x81, 0x00, //     INPUT (Data,Ary,Abs)
 0x25, 0x01, //     LOGICAL_MAXIMUM (1)
 0x95, 0x02, //   REPORT_COUNT (2)
 0x75, 0x01, //   REPORT_SIZE (1)
 0x05, 0x08, //   USAGE_PAGE (LEDs)
 0x19, 0x01, //   USAGE_MINIMUM (Num Lock)
 0x29, 0x02, //   USAGE_MAXIMUM (Caps Lock)
 0x91, 0x02, //   OUTPUT (Data,Var,Abs)
 0x95, 0x01, //   REPORT_COUNT (1)
 0x75, 0x06, //   REPORT_SIZE (6)
 0x91, 0x03, //   OUTPUT (Cnst,Var,Abs)
 0xc0        // END_COLLECTION
};  



/* USB String Descriptors (optional) */
const u8 Joystick_StringLangID[JOYSTICK_SIZ_STRING_LANGID] =
  {
    JOYSTICK_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04
  }
  ; /* LangID = 0x0409: U.S. English */

const u8 Joystick_StringVendor[JOYSTICK_SIZ_STRING_VENDOR] =
  {
    JOYSTICK_SIZ_STRING_VENDOR, /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType*/
    /* Manufacturer: "STMicroelectronics" */
    'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0,
    'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0,
    'c', 0, 's', 0
  };

const uint8_t Joystick_StringProduct[JOYSTICK_SIZ_STRING_PRODUCT] =
  {
    JOYSTICK_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, ' ', 0, 'J', 0,
    'o', 0, 'y', 0, 's', 0, 't', 0, 'i', 0, 'c', 0, 'k', 0
  };
uint8_t Joystick_StringSerial[JOYSTICK_SIZ_STRING_SERIAL] =
  {
    JOYSTICK_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, '1', 0, '0', 0
  };

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/


