// base headers
#include "mew.h"
#include "debug.h"
// drivers
#include "drivers/duart/duart.h"
#include "drivers/usb/mew_usb_hid.h"
#include "drivers/bluetooth/bluetooth.h"
#include "drivers/system/system.h"
#include "drivers/spi_flash/flash.h"
#include "drivers/i2c/i2c.h"
#include "drivers/flash/flash.h"
#include "drivers/display/display.h"
#include "drivers/hw_crypt/P256-cortex-ecdh.h"
// application modules
#include "app/packet_parser/parser.h"
// ui modules
#include "ui/ui.h"

static volatile unsigned long _mew_loop_counter = 0;

const mew_driver drivers[] = {
    {
        MEW_DRIVER_ID_CLOCK, 
        "STM32F4 System clock",
        &mew_clock_init, 
        NULL,
    }, {
        MEW_DRIVER_ID_DUART, 
        "\n\n-------- STM32F4 Debug usart started --------",
        &mew_init_duart, 
        NULL,
    }, {
        MEW_DRIVER_ID_SYSTICK, 
        "STM32F4 Systick",
        &mew_systick_init, 
        NULL,
    }, {
        MEW_DRIVER_ID_USB_HID_KB, 
        "USB HID keyboard",
        &mew_hid_usb_init, 
        NULL,
    }, {
		MEW_DRIVER_ID_RNG,
		"Hardware random number generator",
		&mew_rng_init,
		NULL
	}, {
		MEW_DRIVER_ID_P256_ECDH,
		"P-256 ECDH Keypair generator",
		&mew_p256_ecdh_handler,
		&mew_p256_ecdh_test
	}, {
        MEW_DRIVER_ID_BLUETOOTH, 
        "BLE module HM-11",
        &mew_bluetooth_init, 
        NULL
    }, {
        MEW_DRIVER_ID_SPI_FLASH,
        "128 MBytes spi flash",
        &mew_spi_flash_init,
        NULL
    }, {
        MEW_DRIVER_ID_I2C,
        "STM32F4 I2C",
        &mew_i2c_init,
        NULL // &mew_i2c_eeprom_test - this test will corrupt your data in the eeprom
    }, {
		MEW_DRIVER_ID_OTP_STORAGE,
		"MeW ID OTP storage",
		&mew_otp_storage_init,
		NULL
	}, {
    	MEW_DRIVER_ID_SEC_EEPROM,
		"Encrypted eeprom for settings",
		&mew_load_secure_settings,
		NULL
	}, {
        MEW_DRIVER_ID_DISPLAY,
        "2.4' TFT SPI display",
        &mew_display_init,
        NULL //&mew_ui_show_pinpad
    }, {
        MEW_DRIVER_ID_TOUCHSCREEN,
        "Resistive touchscreen with hardware controller",
        &mew_touchscreen_init,
        NULL
    }, {
        0
    }
};

const mew_loop_handler loop_handlers[] = {
    {
			MEW_LHANDLER_ID_PACKAGE_PARSER,
			"Stream to packages parser",
			&mew_comm_handler
    }, /*{
    		MEW_LHANDLER_ID_BLUETOOTH,
			"Bluetooth at-terminal",
			&mew_bluetooth_handler
    },*/ {
    		MEW_LHANDLER_ID_UI,
			"UI logic handle",
			&mew_ui_lhandler
	}, {
			MEW_LHANDLER_ID_LVGL,
			"LVGL handle",
			&mew_lvgl_lhandler
    }, {
    		MEW_LHANDLER_ID_SPI_FLASH,
			"SPI Flash async read/write handler",
			&mew_spi_flash_handler
	}, {
        0
    }
};

int main(void) {
    unsigned int i = 0;
    const mew_driver *drv;
    const mew_loop_handler *loop_handler;
    
    // init all drivers
    while(1) {
        drv = &drivers[i];
        if (drv->id == 0) {
            break;
        }

        if (drv->init() != 0) {
            mew_debug_die_with_message(drv->display_name);
        } else {
            mew_debug_print(drv->display_name);
        }
        
        i++;
    }
    
    // devices test 
    i = 0;
    while (1) {
        drv = &drivers[i];
        if (drv->id == 0) {
            break;
        }
        
        if (drv->test != NULL) {
            drv->test();
        }
        
        i++;
    }

    // main loop
    while(1) {
        i = 0;
        while(1) {
            loop_handler = &loop_handlers[i];
            if (loop_handler->id == 0) {
                break;
            }
            
            if (loop_handler->handler() != 0) {
                mew_debug_die_with_message(loop_handler->display_name);
            }
            
            i++;
        }
        _mew_loop_counter++;
    }
    return 1;
}
