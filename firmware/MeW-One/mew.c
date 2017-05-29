#include <libopencm3/stm32/rcc.h>
#include "board.h"
#include "usb.h"

void button_click(void);

volatile unsigned int
			buttonState = 0,
			buttonLock = 0;

int main(void) {
	mew_board_init();
	mew_led_on(MEW_LED_READY);

	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	rcc_wait_for_osc_ready(RCC_HSE);
	mew_led_off(MEW_LED_READY);

	mew_timer2_init();
	mew_init_ei();

    mew_usb_init();
	while(1)
		mew_usb_poll();

	return 1;
}

void tim2_isr(void) {
	if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {
		timer_clear_flag(TIM2, TIM_SR_CC1IF);
		if (mew_read_buttons() == 0) {
			mew_led_off(MEW_LED_READY);
			buttonState = 0;
		}
	}
}

void button_click(void) {
	mew_led_on(MEW_LED_READY);
	timer_set_counter(TIM2, 0);
	mew_send_char(16, MEW_LOWER_CASE);
}

