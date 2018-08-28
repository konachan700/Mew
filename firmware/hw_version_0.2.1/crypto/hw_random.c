#include "crypto.h"

static volatile uint32_t _mew_last_random_value = 0;

void mew_start_random(void) {
    RNG_CR |= RNG_CR_IE;
    RNG_CR |= RNG_CR_RNGEN;
}

uint32_t mew_random32(void) {
    uint32_t random = _mew_last_random_value;
	while (_mew_last_random_value == random) {
		if (((RNG_SR & (RNG_SR_SEIS | RNG_SR_CEIS)) == 0) && ((RNG_SR & RNG_SR_DRDY) == 1)) {
            random = RNG_DR;
        }
	}
	
	_mew_last_random_value = random;
    return _mew_last_random_value;
}