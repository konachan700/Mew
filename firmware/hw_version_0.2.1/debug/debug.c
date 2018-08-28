#include "../debug/debug.h"

char __mew_to_hex(char in);

void mew_led_init(void) {
    gpio_mode_setup(MEW_LED_PORT, GPIO_MODE_OUTPUT, GPIO_MODE_OUTPUT, MEW_LED_PIN);
    gpio_clear(MEW_LED_PORT, MEW_LED_PIN);
}

void mew_led_toggle(void) {
    gpio_toggle(MEW_LED_PORT, MEW_LED_PIN);
}

void mew_start_debug_usart(void) {
    gpio_mode_setup(MEW_DEBUG_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MEW_DEBUG_PIN);
    gpio_set_af(MEW_DEBUG_PORT, MEW_DEBUG_PORT_AF, MEW_DEBUG_PIN);
    
    usart_set_baudrate(MEW_DEBUG_USART, MEW_DEBUG_SPEED);
    usart_set_databits(MEW_DEBUG_USART, 8);
    usart_set_stopbits(MEW_DEBUG_USART, USART_STOPBITS_1);
    usart_set_mode(MEW_DEBUG_USART, USART_MODE_TX);
    usart_set_parity(MEW_DEBUG_USART, USART_PARITY_NONE);
    usart_set_flow_control(MEW_DEBUG_USART, USART_FLOWCONTROL_NONE);
    usart_enable(MEW_DEBUG_USART);
}

void mew_debug_print(const char* text) {
    uint16_t i = 0;
    while (1) {
        if (text[i] == 0) {
            usart_send_blocking(MEW_DEBUG_USART, '\r');
            usart_send_blocking(MEW_DEBUG_USART, '\n');
            return;
        }
        usart_send_blocking(MEW_DEBUG_USART, text[i]);
        i++;
    }
} 

char __mew_to_hex(char in) {
	char buf = in & 0x0F;
	switch (buf) {
	case 0:
		return '0';
	case 1:
		return '1';
	case 2:
		return '2';
	case 3:
		return '3';
	case 4:
		return '4';
	case 5:
		return '5';
	case 6:
		return '6';
	case 7:
		return '7';
	case 8:
		return '8';
	case 9:
		return '9';
	case 0x0A:
		return 'A';
	case 0x0B:
		return 'B';
	case 0x0C:
		return 'C';
	case 0x0D:
		return 'D';
	case 0x0E:
		return 'E';
	case 0x0F:
		return 'F';
	}
	return '0';
}

void mew_debug_print_hex(uint8_t* blob, uint16_t len) {
    uint16_t i = 0, nl = 0;
    for (i=0; i<len; i++) {
        usart_send_blocking(USART2, __mew_to_hex(blob[i] >> 4));
        usart_send_blocking(USART2, __mew_to_hex(blob[i]));
        usart_send_blocking(USART2, ' ');
        
        nl++;
        if (nl >= 16) {
            nl = 0;
            usart_send_blocking(USART2, '\r');
            usart_send_blocking(USART2, '\n');
        }
    }
        
    usart_send_blocking(USART2, '\r');
    usart_send_blocking(USART2, '\n');
}
