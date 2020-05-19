#ifndef __MEW_STATE__
#define __MEW_STATE__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef NULL
#define NULL 0
#endif

#define __MEW_FACTORY_TESTS__

#define APP_ADDRESS 					0x08020000

#define MEW_DRIVER_ID_DUART             0x01
#define MEW_DRIVER_ID_SYSTICK           0x02
#define MEW_DRIVER_ID_CLOCK             0x03

#define MEW_DRIVER_ID_USB_HID_KB        0x10
#define MEW_DRIVER_ID_BLUETOOTH         0x11
#define MEW_DRIVER_ID_SPI_FLASH         0x12
#define MEW_DRIVER_ID_I2C               0x13
#define MEW_DRIVER_ID_RNG               0x14
#define MEW_DRIVER_ID_OTP_STORAGE       0x15
#define MEW_DRIVER_ID_TOUCHSCREEN       0x16
#define MEW_DRIVER_ID_DISPLAY           0x17
#define MEW_DRIVER_ID_P256_ECDH			0x18
#define MEW_DRIVER_ID_SEC_EEPROM        0x19

#define MEW_LHANDLER_ID_PACKAGE_PARSER  0xA0
#define MEW_LHANDLER_ID_LVGL            0xA1
#define MEW_LHANDLER_ID_SPI_FLASH       0xA2
#define MEW_LHANDLER_ID_UI              0xA3
//#define MEW_LHANDLER_ID_BLUETOOTH       0xA4

typedef struct {
    unsigned int id;
    const char* display_name;
    unsigned int (*init)(void);
    unsigned int (*test)(void);
} mew_driver;

typedef struct {
    unsigned int id;
    const char* display_name;
    unsigned int (*handler)(void);
} mew_loop_handler;

#endif
