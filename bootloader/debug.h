#ifndef MEW_DEBUG_H
#define MEW_DEBUG_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void mew_debug_print(const char* message);
void mew_debug_print_hex(const char* message, unsigned int len);
void mew_debug_print_hex_ex(const char* text, const char* blob, unsigned int len);

void mew_debug_die(void);
void mew_debug_die_with_message(const char* message);

#endif /* DEBUG_H */

