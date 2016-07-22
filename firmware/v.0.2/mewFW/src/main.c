#include "stm32f10x_conf.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "board_config.h"

#define MEW_ANY_BTN_PRESSED 1
#define MEW_ANY_BTN_NO_PRESSED 0

MEW_BUTTONS mew_btns;
volatile int is_any_button_pressed = 0;

int main(void) {
	int i;
	for (i=0; i<5000000; i++);

    Set_System();
    SystemInit();

    USB_Interrupts_Config();
    Set_USBClock();
    USB_Init();

    mew_board_init();
    mew_tmr3_init();

    while(1) {
        mew_read_buttons(&mew_btns);
        if (is_any_button_pressed == MEW_ANY_BTN_NO_PRESSED) {
            if ((mew_btns.button_1      == MEW_BUTTON_PRESSED) ||
                (mew_btns.button_2      == MEW_BUTTON_PRESSED) ||
                (mew_btns.button_3      == MEW_BUTTON_PRESSED) ||
                (mew_btns.button_4      == MEW_BUTTON_PRESSED) ||
                (mew_btns.button_select == MEW_BUTTON_PRESSED) ) {
                    is_any_button_pressed = MEW_ANY_BTN_PRESSED;
                    mew_tmr3_reset_counter();
                }
        }

        for (i=0; i<5000000; i++);
    }
}

void TIM3_IRQHandler(void) {
    if (is_any_button_pressed == MEW_ANY_BTN_PRESSED) {


        mew_clear_buttons(&mew_btns);
        is_any_button_pressed = MEW_ANY_BTN_NO_PRESSED;
    }

	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
}

