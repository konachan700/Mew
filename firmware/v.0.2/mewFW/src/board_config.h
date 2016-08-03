#ifndef _CONFIG_MEW
    #include "stm32f10x_conf.h"

    #define _CONFIG_MEW
    #define MEW_HW_VERSION_2

    #ifdef MEW_HW_VERSION_2
        #define MEW_BUTTONS_PORT        GPIOB

        #define MEW_BUTTON_01_PIN       GPIO_Pin_8
        #define MEW_BUTTON_02_PIN       GPIO_Pin_9
        #define MEW_BUTTON_03_PIN       GPIO_Pin_10
        #define MEW_BUTTON_04_PIN       GPIO_Pin_11
        #define MEW_BUTTON_MODE_PIN     GPIO_Pin_12

        #define MEW_LED_PORT            GPIOA

        #define MEW_LED_R_PIN           GPIO_Pin_1
        #define MEW_LED_G_PIN           GPIO_Pin_2
        #define MEW_LED_B_PIN           GPIO_Pin_0
        #define MEW_LED_PWR_PIN         GPIO_Pin_3

        typedef struct {
            volatile char button_1;
            volatile char button_2;
            volatile char button_3;
            volatile char button_4;
            volatile char button_select;
        } MEW_BUTTONS;
    #endif

    #define MEW_BUTTON_PRESSED 1
    #define MEW_BUTTON_UNPRESSED 0

    extern void mew_board_init(void);

    extern void mew_read_buttons(MEW_BUTTONS* buttons);
    extern void mew_clear_buttons(MEW_BUTTONS* buttons);

    extern void mew_led_on(uint16_t GPIO_Pin);
    extern void mew_led_off(uint16_t GPIO_Pin);

    extern void mew_tmr3_init(void);
    extern void mew_tmr3_reset_counter(void);
#endif
