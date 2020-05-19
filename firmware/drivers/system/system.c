#include "debug.h"
#include "system.h"

void mew_check_dma_memory(void* pointer, char* where) {
    uint32_t p = ((uint32_t) pointer);
    //mew_debug_print_hex((uint8_t*) &pointer, 4);
    if ((p > 0x10000000) && (p < 11000000)) {
        mew_debug_print(where);
        mew_debug_die_with_message("Illegal memory access!");
    }
}

void mew_wait_for_state(volatile uint8_t* var, uint8_t expected_state) {
    uint32_t counter;
    for (counter=0; counter<1000000; counter++) {
        if (*var == expected_state) return;
    }
    mew_debug_print("too long wait...");
    mew_debug_die();
}

