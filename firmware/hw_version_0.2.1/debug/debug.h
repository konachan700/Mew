#ifndef MEW_DEBUG
#define MEW_DEBUG

#include "../config.h" 

#define MEW_DEBUG_USART         USART2
#define MEW_DEBUG_PORT          GPIOA
#define MEW_DEBUG_PORT_AF       GPIO_AF7
#define MEW_DEBUG_PIN           GPIO2
#define MEW_DEBUG_SPEED         115200

#define MEW_LED_PORT            GPIOB
#define MEW_LED_PIN             GPIO0

#ifdef INTEGRATION_TESTS
void mew_integration_test(void);
#else
#define mew_integration_test
#endif

extern void mew_start_debug_usart(void);
extern void mew_debug_print(const char* text);
extern void mew_debug_print_hex(uint8_t * blob, uint16_t len);

void mew_led_init(void);
void mew_led_toggle(void);

#endif
