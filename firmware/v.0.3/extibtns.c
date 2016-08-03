#include "board.h"
#include "extibtns.h"

extern void button_click(void);

extern volatile unsigned int buttonState;

#ifdef MEW_HW_VERSION_2
void exti9_5_isr(void) {
	if (EXTI_PR | EXTI8) {
		if (buttonState == 0) {
			buttonState |= MEW_BUTTON(0);
			button_click();
		}
		exti_reset_request(EXTI8);
	}

	if (EXTI_PR | EXTI9) {
		if (buttonState == 0) {
			buttonState |= MEW_BUTTON(1);
			button_click();
		}
		exti_reset_request(EXTI9);
	}
}

void exti15_10_isr(void) {
	if (EXTI_PR | EXTI10) {
			if (buttonState == 0) {
				buttonState |= MEW_BUTTON(2);
				button_click();
			}
			exti_reset_request(EXTI10);
	}

	if (EXTI_PR | EXTI11) {
		if (buttonState == 0) {
			buttonState |= MEW_BUTTON(3);
			button_click();
		}
		exti_reset_request(EXTI11);
	}

	if (EXTI_PR | EXTI12) {
		if (buttonState == 0) {
			buttonState |= MEW_BUTTON_MODE;
			button_click();
		}
		exti_reset_request(EXTI12);
	}
}
#endif

#ifdef MEW_HW_VERSION_1
void exti2_isr(void) {
	if (buttonState == 0) {
		buttonState |= MEW_BUTTON(4);
		button_click();
	}
	exti_reset_request(EXTI2);
}

void exti4_isr(void) {
	if (buttonState == 0) {
		buttonState |= MEW_BUTTON_MODE;
		button_click();
	}
	exti_reset_request(EXTI4);
}

void exti9_5_isr(void) {
	if (EXTI_PR | EXTI8) {
		if (buttonState == 0) {
			buttonState |= MEW_BUTTON(3);
			button_click();
		}
		exti_reset_request(EXTI8);
	}

	if (EXTI_PR | EXTI9) {
		if (buttonState == 0) {
			buttonState |= MEW_BUTTON(2);
			button_click();
		}
		exti_reset_request(EXTI9);
	}
}

void exti15_10_isr(void) {
	if (EXTI_PR | EXTI11) {
		if (buttonState == 0) {
			buttonState |= MEW_BUTTON(1);
			button_click();
		}
		exti_reset_request(EXTI11);
	}

	if (EXTI_PR | EXTI12) {
		if (buttonState == 0) {
			buttonState |= MEW_BUTTON(0);
			button_click();
		}
		exti_reset_request(EXTI12);
	}
}
#endif
