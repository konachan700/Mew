#ifndef MEW_DUART
#define MEW_DUART

#include "mew.h" 

#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>

#include <stdio.h>

#define MEW_DEBUG_USART         USART2
#define MEW_DEBUG_PORT          GPIOA
#define MEW_DEBUG_PORT_AF       GPIO_AF7
#define MEW_DEBUG_PIN           GPIO2
#define MEW_DEBUG_SPEED         115200

unsigned int mew_init_duart(void);

void mew_duart_print(const char* text);
void mew_duart_print_ex(const char* text, unsigned int newline);
void mew_duart_print_hex(const char * blob, unsigned int len);
void mew_duart_print_hex_ex(const char* text, const char* blob, unsigned int len);

#endif
