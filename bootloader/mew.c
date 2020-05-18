// base headers
#include "mew.h"
#include "debug.h"
// drivers
#include "drivers/duart/duart.h"
#include "drivers/system/system.h"
#include "drivers/i2c/i2c.h"
#include "drivers/display/display.h"
#include "drivers/usb/mew_usb_hid.h"

static volatile unsigned long _mew_loop_counter = 0;
extern volatile uint8_t is_bootloader_active;

const mew_driver drivers[] = {
    {
        MEW_DRIVER_ID_CLOCK, 
        "STM32F4 System clock",
        &mew_clock_init, 
        NULL,
    }, {
        MEW_DRIVER_ID_DUART, 
        "\n\n STM32F4 Debug usart",
        &mew_init_duart, 
        NULL,
    }, {
        MEW_DRIVER_ID_SYSTICK, 
        "STM32F4 Systick",
        &mew_systick_init, 
        NULL,
    }, {
        MEW_DRIVER_ID_I2C,
        "STM32F4 I2C",
        &mew_i2c_init,
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
		&mew_touchscreen_test
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

    if (is_bootloader_active == 1) {
    	mew_hid_usb_init();
    	mew_debug_print("MeW is in firmware update mode");
    	while(1) {


    	}
    }

    // TODO: jump to real code
    while(1) {}
    return 1;
}
