/*
 * hal_stm32f1_delay.c
 *
 *  Created on: Aug 26, 2016
 *      Author: misaki
 */

#include "atca_hal.h"
#include "delay.h"

#define CPU_F 72000000

void atca_delay_us(uint32_t delay)
{
	delay_us(delay, CPU_F);
}

void atca_delay_10us(uint32_t delay)
{
	delay_us(delay * 10, CPU_F);

}

void atca_delay_ms(uint32_t delay)
{
	delay_ms(delay, CPU_F);
}
