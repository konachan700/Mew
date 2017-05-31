#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/assert.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/f2/rng.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/cortex.h>

#include "ui.h"
#include "ILI9341.h"
#include "menu.h"
#include "board.h" 
#include "usb_hid.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>  

volatile u8 button = BUTTON_NONE;
volatile u8 is_any_button_pressed = 0;

u8 __read_buttons(void) {
    if (MEW_BUTTON_UP_PRESSED)      return BUTTON_UP;
    if (MEW_BUTTON_DOWN_PRESSED)    return BUTTON_DOWN;
    if (MEW_BUTTON_OK_PRESSED)      return BUTTON_OK;
    if (MEW_BUTTON_BACK_PRESSED)    return BUTTON_BACK;
    return 0xff;
}

void __button_set(u8 b) {
    if (button == BUTTON_NONE) {
        button = b;
        timer_set_counter(TIM2, 0);
    }
}

void exti15_10_isr(void) {    
    if (EXTI_PR & EXTI15) {
        __button_set(BUTTON_UP);
		exti_reset_request(EXTI15);
	}
    
	if (EXTI_PR & EXTI12) {
        __button_set(BUTTON_OK);
        exti_reset_request(EXTI12);
	}

	if (EXTI_PR & EXTI13) {
        __button_set(BUTTON_DOWN);
		exti_reset_request(EXTI13);
	}

	if (EXTI_PR & EXTI14) {
        __button_set(BUTTON_BACK);
		exti_reset_request(EXTI14);
	}
}

void tim2_isr(void) {
	if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {
		timer_clear_flag(TIM2, TIM_SR_CC1IF);
        gpio_toggle(GPIOC, GPIO3);
        
        if (button != BUTTON_NONE) {
            if (__read_buttons() == button) is_any_button_pressed = button;
            button = BUTTON_NONE;
        }
	}
}

int main(void) {   
    start_all_clock();
    start_debug_usart();
    debug_print("usart started.");
    
    start_leds();
    start_backlight();
    start_timer_2();
    start_buttons();
    start_i2c1();
    
    start_spi_2_non_dma();
    display_setup();
    start_spi_2_dma();
    display_fill(0, 0, 0);
    statusbar_paint();
    draw_root_menu();
    
    u32 ret;
    u8 data[64];
    
    ret = i2c_fram_write_dma(0, 0x10, (u8[]){1,2,3,4}, 4);
    debug_print("RD I2C: ");
    debug_print_hex((u8[]) {ret >> 8, ret}, 2);
    
    ret = i2c_fram_read_dma(0, 0x00, data, 64);
    debug_print_hex(data, 64);
    debug_print("RD I2C: ");
    debug_print_hex((u8[]) {ret >> 8, ret}, 2);
    


    mew_hid_usb_init();

	while(1) {
        if (is_any_button_pressed != 0) {
            button_pressed(is_any_button_pressed);
            is_any_button_pressed = 0;
        }
        mew_hid_usb_poll();
    }
    
	return 1;
}

