/*
 * delay.h
 *
 *  Created on: Aug 26, 2016
 *      Author: misaki
 */

#ifndef DELAY_H_
#define DELAY_H_

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

extern inline void delay_cycles(const int64_t cycles);
extern inline void delay_us(uint32_t us, uint64_t cpufreq);
extern inline void delay_ms(uint32_t ms, uint64_t cpufreq);

#endif /* DELAY_H_ */
