/*
 * This file is part of the HAL project, inline library above libopencm3.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @defgroup DELAY_module DELAY module
 *
 * @brief Spin-waiting blocking API
 *
 * @ingroup modules
 *
 * LGPL License Terms @ref lgpl_license
 */

/*****************************************************************************/
/* Architecture dependent implementations                                    */
/*****************************************************************************/

#include "delay.h"

static void _delay_3t(uint32_t cycles) __attribute__((naked));

/* 3 Tcyc per tick, 4Tcyc call/ret, 1Tcyc hidden reg assignment */
static void _delay_3t(uint32_t cycles)
{
	asm __volatile__ (
		"1: \n"
		"	subs %[cyc],#1 \n"	/* 1Tck */
		"	bne 1b \n"		/* 2Tck */
		"	bx lr \n"
		: /* No output */
		: [cyc] "r" (cycles)
		: /* No memory */
	);
}


inline void delay_cycles(const int64_t cycles)
{
	if (cycles <= 0)
		return;

	switch (cycles % 3) {
	default:
	case 0: break;
	case 1: asm __volatile__ ("nop"); break;
	case 2: asm __volatile__ ("nop\nnop"); break;
	}

	if (cycles > 3)
		_delay_3t((uint32_t)(cycles / 3));
	else /* same delay as the function call */
		asm __volatile__ ("nop\nnop\nnop\nnop\nnop\nnop\n");
}


/* max 25 sec @ 168MHz! */
/* max 525 sec @ 8MHz! */
inline void delay_us(uint32_t us, uint64_t cpufreq)
{
	if (us == 0) return;
	delay_cycles(us * cpufreq / 1000000 - 6);
}

/* max 25 sec @ 168MHz! */
/* max 525 sec @ 8MHz! */
inline void delay_ms(uint32_t ms, uint64_t cpufreq)
{
	if (ms == 0)
		return;

	delay_cycles(ms * cpufreq / 1000 - 6);
}

