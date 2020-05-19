#include "debug.h"
#include "drivers/duart/duart.h"

static void debug_blink(void);

void mew_debug_die(void) {
    uint32_t counter;
    __asm__("cpsid i");
    while (1) {
        for (counter=0; counter<20000000; counter++) {
            __asm__("NOP");
        }
        debug_blink();
    }
}

void mew_debug_die_with_message(const char* message) {
    mew_debug_print(message);
    mew_debug_die();
}

void mew_debug_print(const char* message) {
    mew_duart_print(message);
}

void mew_debug_print_hex(const char* message, unsigned int len) {
    mew_duart_print_hex(message, len);
}

void mew_debug_print_hex_ex(const char* text, const char* blob, unsigned int len) {
    mew_duart_print_hex_ex(text, blob, len);
}

static void debug_blink(void) {
    
}

void hard_fault_handler(void) {
    mew_debug_die_with_message("hard_fault_handler");
}

void usage_fault_handler(void) {
    mew_debug_die_with_message("usage_fault_handler");
}

void bus_fault_handler(void) {
    mew_debug_die_with_message("bus_fault_handler");
}
