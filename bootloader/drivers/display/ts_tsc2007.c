#include "display.h" 
#include "debug.h"
#include "../i2c/i2c.h"

static bool __mew_input_read(int *x, int *y, int *state);

static volatile uint8_t _mew_ts_isr = 0;
volatile uint8_t is_bootloader_active = 0;

unsigned int mew_touchscreen_test(void) {
	int x=0, y=0, state=0;
	__mew_input_read(&x, &y, &state);
	if ((x > 16) && (x < 64) && (y > 16) && (y < 64) && (state == 1)) {
		is_bootloader_active = 1;
	}
	return 0;
}

unsigned int mew_touchscreen_init(void) {
    gpio_mode_setup(MEW_TSC2007_IRQ_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, MEW_TSC2007_IRQ_PIN); 
    gpio_set(MEW_TSC2007_IRQ_PORT, MEW_TSC2007_IRQ_PIN);
    nvic_enable_irq(MEW_TSC2007_IRQ_NVIC);
    exti_select_source(MEW_TSC2007_IRQ_EXTI, MEW_TSC2007_IRQ_PORT);
    exti_set_trigger(MEW_TSC2007_IRQ_EXTI, EXTI_TRIGGER_FALLING);
    exti_enable_request(MEW_TSC2007_IRQ_EXTI);
    
    uint8_t res = mew_i2c_write(0x48, MEW_TSC2007_PWRDOWN, 0x00, MEW_I2C_MODE_0BIT);
    if (res == 1) {


    } else {
        mew_debug_die_with_message("mew_touchscreen_init: cannot init ts");
    }
    
    return 0;
}

void MEW_TSC2007_IRQ_ISR(void) {
    exti_reset_request(MEW_TSC2007_IRQ_EXTI);
    _mew_ts_isr = 1;
}

static bool __mew_input_read(int *out_x, int *out_y, int *state) {
    float xf, yf;
    uint16_t x, y, z;
    uint8_t res;
    
    if (_mew_ts_isr == 0) {
        return false;
    }
    
    res = mew_i2c_read_block_ts2007(0x48, MEW_TSC2007_READ_Z1, (uint8_t*) &z, 2);
    if (res == 0) {
        mew_debug_die_with_message("mew_touchscreen_init: cannot read Z");
    }
    
    z = MEW_12BIT_SWAP(z);
    if (z > 0x100) {
        res = mew_i2c_read_block_ts2007(0x48, MEW_TSC2007_READ_X, (uint8_t*) &x, 2);
        if (res == 0) {
            mew_debug_die_with_message("mew_touchscreen_init: cannot read X");
        }

        res = mew_i2c_read_block_ts2007(0x48, MEW_TSC2007_READ_Y, (uint8_t*) &y, 2);
        if (res == 0) {
            mew_debug_die_with_message("mew_touchscreen_init: cannot read Y");
        }
    
        xf = MEW_12BIT_SWAP(x);
        xf = (xf / 0x1000) * MEW_DISPLAY_W;

        yf = MEW_12BIT_SWAP(y);
        yf = (yf / 0x1000) * MEW_DISPLAY_H;


        state[0] = 1;
        out_x[0] = (uint16_t) (xf);
        out_y[0] = (uint16_t) (yf);
    } else {
        state[0] = 0;
    }

    res = mew_i2c_write(0x48, MEW_TSC2007_PWRDOWN, 0x00, MEW_I2C_MODE_0BIT);
    
    _mew_ts_isr = 0;
    return false;
}
