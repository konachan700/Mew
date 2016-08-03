#include "stm32f10x_conf.h"
#include "usb_istr.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "platform_config.h"
#include "board_config.h"

void mew_board_init(void) {
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);

    gpio.GPIO_Pin = MEW_BUTTON_01_PIN | MEW_BUTTON_02_PIN | MEW_BUTTON_03_PIN | MEW_BUTTON_04_PIN | MEW_BUTTON_MODE_PIN;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(MEW_BUTTONS_PORT, &gpio);

    gpio.GPIO_Pin = MEW_LED_R_PIN | MEW_LED_G_PIN | MEW_LED_B_PIN | MEW_LED_PWR_PIN;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(MEW_LED_PORT, &gpio);

    GPIO_SetBits(MEW_LED_PORT, MEW_LED_R_PIN | MEW_LED_G_PIN | MEW_LED_B_PIN | MEW_LED_PWR_PIN);
}

void mew_tmr3_init(void) {
	TIM_TimeBaseInitTypeDef t3init;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	t3init.TIM_Prescaler = 7200;
	t3init.TIM_CounterMode = TIM_CounterMode_Up;
	t3init.TIM_Period = 1000;
	t3init.TIM_ClockDivision = 0x0;
	t3init.TIM_RepetitionCounter = 0x0;

	TIM_TimeBaseInit(TIM3, &t3init);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_ClearFlag(TIM3, TIM_IT_Update);
	TIM_Cmd(TIM3, ENABLE);
}

void mew_tmr3_reset_counter(void) {
    TIM3->CNT = 0;
}

void mew_read_buttons(MEW_BUTTONS* buttons) {
    buttons->button_1        = ((MEW_BUTTONS_PORT->IDR & MEW_BUTTON_01_PIN)  == 0)      ? MEW_BUTTON_PRESSED : MEW_BUTTON_UNPRESSED;
    buttons->button_2        = ((MEW_BUTTONS_PORT->IDR & MEW_BUTTON_02_PIN)  == 0)      ? MEW_BUTTON_PRESSED : MEW_BUTTON_UNPRESSED;
    buttons->button_3        = ((MEW_BUTTONS_PORT->IDR & MEW_BUTTON_03_PIN)  == 0)      ? MEW_BUTTON_PRESSED : MEW_BUTTON_UNPRESSED;
    buttons->button_4        = ((MEW_BUTTONS_PORT->IDR & MEW_BUTTON_04_PIN)  == 0)      ? MEW_BUTTON_PRESSED : MEW_BUTTON_UNPRESSED;
    buttons->button_select   = ((MEW_BUTTONS_PORT->IDR & MEW_BUTTON_MODE_PIN)  == 0)    ? MEW_BUTTON_PRESSED : MEW_BUTTON_UNPRESSED;
}

void mew_clear_buttons(MEW_BUTTONS* buttons) {
    buttons->button_1        = MEW_BUTTON_UNPRESSED;
    buttons->button_2        = MEW_BUTTON_UNPRESSED;
    buttons->button_3        = MEW_BUTTON_UNPRESSED;
    buttons->button_4        = MEW_BUTTON_UNPRESSED;
    buttons->button_select   = MEW_BUTTON_UNPRESSED;
}

void mew_led_on(uint16_t GPIO_Pin) {
    GPIO_ResetBits(MEW_LED_PORT, GPIO_Pin);
}

void mew_led_off(uint16_t GPIO_Pin) {
    GPIO_SetBits(MEW_LED_PORT, GPIO_Pin);
}


