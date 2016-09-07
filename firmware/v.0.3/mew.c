#include <libopencm3/stm32/rcc.h>
#include "board.h"
#include "usb.h"
#include "passwords.h"
#include "cryptoauthlib.h"

#define BUTTON_RESET_COUNTER_MAX 100

void button_click(void); // see extibtns.c

volatile unsigned int
			device_mode = 0,
			buttonState = 0, // see extibtns.c
			mode_change_lock = 0,
			timer_counter_btn_reset = 0;

uint8_t serial_number[32];// = {0xFF, 0xDE, 0x43, 0x01};

int main(void) {
	uint16_t i;
	for (i=0; i<32; i++) serial_number[i] = 0x00;

	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	rcc_wait_for_osc_ready(RCC_HSE);

	mew_board_init();
	if (atcab_init(&cfg_sha204a_i2c_default) != ATCA_SUCCESS) while(1);
	//if (atcab_random(serial_number) != ATCA_SUCCESS) while(1);

	mew_led_on(MEW_LED_READY);

	mew_inital_fill_passwords();
	mew_timer2_init();
	mew_init_ei();
    mew_usb_init();

    mew_led_off(MEW_LED_READY);

	while(1)
		mew_usb_poll();

	return 1;
}

void tim2_isr(void) {
	if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {
		timer_clear_flag(TIM2, TIM_SR_CC1IF);
		timer_counter_btn_reset++;
		if (timer_counter_btn_reset > BUTTON_RESET_COUNTER_MAX) {
			if (mew_read_buttons() == 0) {
				mew_led_off(MEW_LED_READY);
				buttonState = 0;
				mode_change_lock = 0;
			}
			timer_counter_btn_reset = 0;
		}

		if (timer_counter_btn_reset == 0) {
			switch (device_mode) {
			case 0:
				mew_led_on(MEW_LED_GREEN);
				break;
			case 1:
				mew_led_on(MEW_LED_RED);
				break;
			case 2:
				mew_led_on(MEW_LED_BLUE);
				break;
#ifdef DEVICE_MODES_7
			case 6:
				mew_led_on(MEW_LED_GREEN);
				mew_led_on(MEW_LED_BLUE);
				break;
			case 4:
				mew_led_on(MEW_LED_GREEN);
				mew_led_on(MEW_LED_RED);
				break;
			case 5:
				mew_led_on(MEW_LED_BLUE);
				mew_led_on(MEW_LED_RED);
				break;

#endif
			case 3:
				mew_led_on(MEW_LED_GREEN);
				mew_led_on(MEW_LED_BLUE);
				mew_led_on(MEW_LED_RED);
				break;
			}
		}

		if (timer_counter_btn_reset == 23) {
			mew_led_off(MEW_LED_GREEN);
			mew_led_off(MEW_LED_BLUE);
			mew_led_off(MEW_LED_RED);
		}
	}
}

void button_click(void) {
	ATCA_STATUS st;

	mew_led_on(MEW_LED_READY);
	timer_set_counter(TIM2, 0);
	if (mode_change_lock == 0) {
		if (buttonState & MEW_BUTTON_MODE) {
			device_mode++;
#ifdef DEVICE_MODES_7
			if (device_mode >= 7) device_mode = 0;
#endif
#ifdef DEVICE_MODES_4
			if (device_mode >= 4) device_mode = 0;
#endif
			mode_change_lock = 1;
		}

		if (buttonState & MEW_BUTTON(0)) {
			mode_change_lock = 1;
			mew_password_send(device_mode, 0);
		}
		if (buttonState & MEW_BUTTON(1)) {
			mode_change_lock = 1;
			mew_password_send(device_mode, 1);
		}
		if (buttonState & MEW_BUTTON(2)) {
			mode_change_lock = 1;
			mew_password_send(device_mode, 2);
		}
		if (buttonState & MEW_BUTTON(3)) {
			mode_change_lock = 1;
			//atcab_random(serial_number);
			//mew_password_send(device_mode, 3);
			st = atcab_read_serial_number(serial_number);
			if (st != ATCA_SUCCESS) {
				serial_number[0] = 0xEE;
				serial_number[1] = st & 0xff;
			}
			mew_send_debug_hex(serial_number, 32);
		}

	}
}

