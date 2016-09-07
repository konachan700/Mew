#include "board.h"
 
void mew_board_init(void) {
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
      
#ifdef MEW_HW_VERSION_2
	gpio_set_mode(MEW_BUTTONS_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
			MEW_BUTTON_01_PIN | MEW_BUTTON_02_PIN | MEW_BUTTON_03_PIN | MEW_BUTTON_04_PIN | MEW_BUTTON_MODE_PIN);
	gpio_set(MEW_BUTTONS_PORT, MEW_BUTTON_01_PIN | MEW_BUTTON_02_PIN | MEW_BUTTON_03_PIN | MEW_BUTTON_04_PIN | MEW_BUTTON_MODE_PIN);
	gpio_set_mode(MEW_LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, MEW_LED_R_PIN | MEW_LED_G_PIN | MEW_LED_B_PIN | MEW_LED_PWR_PIN);
	gpio_set(MEW_LED_PORT, MEW_LED_R_PIN | MEW_LED_G_PIN | MEW_LED_B_PIN | MEW_LED_PWR_PIN);
#endif
#ifdef MEW_HW_VERSION_1
	gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, 0);
	gpio_set_mode(MEW_BUTTONS_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
			MEW_BUTTON_01_PIN | MEW_BUTTON_02_PIN | MEW_BUTTON_03_PIN | MEW_BUTTON_04_PIN | MEW_BUTTON_05_PIN | MEW_BUTTON_MODE_PIN);
	gpio_set(MEW_BUTTONS_PORT, MEW_BUTTON_01_PIN | MEW_BUTTON_02_PIN | MEW_BUTTON_03_PIN | MEW_BUTTON_04_PIN | MEW_BUTTON_05_PIN | MEW_BUTTON_MODE_PIN);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO15);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO3);
	gpio_clear(GPIOA, GPIO15);
	gpio_clear(GPIOB, GPIO3);
#endif
}

unsigned int mew_read_buttons(void) {
	unsigned int buttons = 0;
#ifdef MEW_HW_VERSION_2
	if (MEW_BUTTON_01_STATE) buttons 	|= MEW_BUTTON(0);
	if (MEW_BUTTON_02_STATE) buttons 	|= MEW_BUTTON(1);
	if (MEW_BUTTON_03_STATE) buttons 	|= MEW_BUTTON(2);
	if (MEW_BUTTON_04_STATE) buttons 	|= MEW_BUTTON(3);
	if (MEW_BUTTON_MODE_STATE) buttons 	|= MEW_BUTTON_MODE;
#endif
#ifdef MEW_HW_VERSION_1
	if (MEW_BUTTON_01_STATE) buttons 	|= MEW_BUTTON(0);
	if (MEW_BUTTON_02_STATE) buttons 	|= MEW_BUTTON(1);
	if (MEW_BUTTON_03_STATE) buttons 	|= MEW_BUTTON(2);
	if (MEW_BUTTON_04_STATE) buttons 	|= MEW_BUTTON(3);
	if (MEW_BUTTON_05_STATE) buttons 	|= MEW_BUTTON(4);
	if (MEW_BUTTON_MODE_STATE) buttons 	|= MEW_BUTTON_MODE;
#endif
	return buttons;
}

void mew_led_on(unsigned int led) {
#ifdef MEW_HW_VERSION_2
    switch (led) {
    case MEW_LED_GREEN:
        gpio_clear(MEW_LED_PORT, MEW_LED_G_PIN);
    break;
    case MEW_LED_RED:
        gpio_clear(MEW_LED_PORT, MEW_LED_R_PIN);
    break;
    case MEW_LED_BLUE:
        gpio_clear(MEW_LED_PORT, MEW_LED_B_PIN);
    break;
    case MEW_LED_READY:
        gpio_clear(MEW_LED_PORT, MEW_LED_PWR_PIN);
    break;
    }
#endif
#ifdef MEW_HW_VERSION_1
    switch (led) {
    case MEW_LED_READY:
    	gpio_clear(GPIOB, GPIO3);
    break;
    case MEW_LED_RED:
        gpio_clear(GPIOA, GPIO15);
    break;
    }
#endif
}

void mew_led_off(unsigned int led) {
#ifdef MEW_HW_VERSION_2
    switch (led) {
    case MEW_LED_GREEN:
        gpio_set(MEW_LED_PORT, MEW_LED_G_PIN);
    break;
    case MEW_LED_RED:
        gpio_set(MEW_LED_PORT, MEW_LED_R_PIN);
    break;
    case MEW_LED_BLUE:
        gpio_set(MEW_LED_PORT, MEW_LED_B_PIN);
    break;
    case MEW_LED_READY:
        gpio_set(MEW_LED_PORT, MEW_LED_PWR_PIN);
    break;
    }
#endif
#ifdef MEW_HW_VERSION_1
    switch (led) {
    case MEW_LED_READY:
        gpio_set(GPIOB, GPIO3);
    break;
    case MEW_LED_RED:
    	gpio_set(GPIOA, GPIO15);
    break;
    }
#endif
}

void mew_timer2_init(void) {
	rcc_periph_clock_enable(RCC_TIM2);
	nvic_enable_irq(NVIC_TIM2_IRQ);
	nvic_set_priority(NVIC_TIM2_IRQ, 4);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM2, 128); //72
	timer_disable_preload(TIM2);
	timer_continuous_mode(TIM2);
	timer_set_period(TIM2, 1000);
	timer_disable_oc_output(TIM2, TIM_OC1);
	timer_disable_oc_output(TIM2, TIM_OC2);
	timer_disable_oc_output(TIM2, TIM_OC3);
	timer_disable_oc_output(TIM2, TIM_OC4);
	timer_disable_oc_clear(TIM2, TIM_OC1);
	timer_disable_oc_preload(TIM2, TIM_OC1);
	timer_set_oc_slow_mode(TIM2, TIM_OC1);
	timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_FROZEN);
	timer_set_oc_value(TIM2, TIM_OC1, 1000);
	timer_disable_preload(TIM2);
	timer_enable_counter(TIM2);
	timer_enable_irq(TIM2, TIM_DIER_CC1IE);
}

void mew_init_ei(void) {
#ifdef MEW_HW_VERSION_1
	nvic_enable_irq(NVIC_EXTI2_IRQ);
	nvic_enable_irq(NVIC_EXTI4_IRQ);
	nvic_enable_irq(NVIC_EXTI9_5_IRQ);
	nvic_enable_irq(NVIC_EXTI15_10_IRQ);

	exti_select_source(EXTI4 | EXTI2 | EXTI8 | EXTI9 | EXTI11 | EXTI12, GPIOB);
	exti_set_trigger(EXTI4 | EXTI2 | EXTI8 | EXTI9 | EXTI11 | EXTI12, EXTI_TRIGGER_FALLING);
	exti_enable_request(EXTI4 | EXTI2 | EXTI8 | EXTI9 | EXTI11 | EXTI12);
#endif

#ifdef MEW_HW_VERSION_2
	nvic_enable_irq(NVIC_EXTI15_10_IRQ);
	nvic_enable_irq(NVIC_EXTI9_5_IRQ);

	exti_select_source(EXTI8, GPIOB);
	exti_select_source(EXTI9, GPIOB);
	exti_select_source(EXTI10, GPIOB);
	exti_select_source(EXTI11, GPIOB);
	exti_select_source(EXTI12, GPIOB);
	exti_set_trigger(EXTI8 | EXTI9 | EXTI10 | EXTI11 | EXTI12, EXTI_TRIGGER_FALLING);
	exti_enable_request(EXTI8 | EXTI9 | EXTI10 | EXTI11 | EXTI12);
#endif
}

















