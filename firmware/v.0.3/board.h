#ifndef __BOARD_MEW__
#define __BOARD_MEW__

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

//#define MEW_HW_VERSION_2
#define MEW_HW_VERSION_1

#ifdef MEW_HW_VERSION_2
    #define MEW_BUTTONS_PORT        GPIOB

    #define MEW_BUTTON_01_PIN       GPIO8
    #define MEW_BUTTON_02_PIN       GPIO9
    #define MEW_BUTTON_03_PIN       GPIO10
    #define MEW_BUTTON_04_PIN       GPIO11
    #define MEW_BUTTON_MODE_PIN     GPIO12

    #define MEW_LED_PORT            GPIOA

    #define MEW_LED_R_PIN           GPIO1
    #define MEW_LED_G_PIN           GPIO2
    #define MEW_LED_B_PIN           GPIO0
    #define MEW_LED_PWR_PIN         GPIO3

    #define MEW_BUTTON_01_STATE       (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_01_PIN))
    #define MEW_BUTTON_02_STATE       (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_02_PIN))
    #define MEW_BUTTON_03_STATE       (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_03_PIN))
    #define MEW_BUTTON_04_STATE       (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_04_PIN))
    #define MEW_BUTTON_MODE_STATE     (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_MODE_PIN))
#endif

#ifdef MEW_HW_VERSION_1
	#define MEW_BUTTONS_PORT        GPIOB

    #define MEW_BUTTON_01_PIN       GPIO12
    #define MEW_BUTTON_02_PIN       GPIO11
    #define MEW_BUTTON_03_PIN       GPIO9
    #define MEW_BUTTON_04_PIN       GPIO8
	#define MEW_BUTTON_05_PIN       GPIO2
    #define MEW_BUTTON_MODE_PIN     GPIO4


    #define MEW_BUTTON_01_STATE       (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_01_PIN))
    #define MEW_BUTTON_02_STATE       (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_02_PIN))
    #define MEW_BUTTON_03_STATE       (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_03_PIN))
    #define MEW_BUTTON_04_STATE       (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_04_PIN))
	#define MEW_BUTTON_05_STATE       (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_05_PIN))
    #define MEW_BUTTON_MODE_STATE     (!gpio_get(MEW_BUTTONS_PORT, MEW_BUTTON_MODE_PIN))
#endif

#define MEW_BUTTON(n)       (1 << n)
#define MEW_BUTTON_MODE     (1 << 15)

#define MEW_BUTTON_PRESSED 0
#define MEW_BUTTON_UNPRESSED 1

#define MEW_LED_GREEN   1
#define MEW_LED_RED     2
#define MEW_LED_BLUE    3
#define MEW_LED_READY   4

extern void mew_board_init(void);
extern void mew_timer2_init(void);
extern void mew_init_ei(void);

extern unsigned int mew_read_buttons(void);

extern void mew_led_on(unsigned int led);
extern void mew_led_off(unsigned int led);

#endif
