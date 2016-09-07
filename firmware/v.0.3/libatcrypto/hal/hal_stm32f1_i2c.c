/*
 * hal_stm32f1_i2c.c
 *
 *  Created on: Aug 25, 2016
 *      Author: misaki
 */

#include "hal_stm32f1_i2c.h"

ATCA_STATUS hal_i2c_discover_buses(int i2c_buses[], int max_buses)
{
	return ATCA_UNIMPLEMENTED;
}

ATCA_STATUS hal_i2c_discover_devices(int busNum, ATCAIfaceCfg cfg[], int *found )
{
	return ATCA_UNIMPLEMENTED;
}

ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
{
	//SWI2C_init();
	mew_i2c1_init();
	return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
	return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *txdata, int txlength)
{
	enum MEW_I2C_RETVAL st = mew_i2c_write(I2C1, MEW_ATSHA204A_I2C_ADDR, txdata, txlength);
	if (st == GENERAL_ERROR_WRITE)
		return ATCA_TX_FAIL;
	else if (st == GENERAL_ERROR_READ)
		return ATCA_RX_FAIL;
	else
		return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength)
{
	enum MEW_I2C_RETVAL st = mew_i2c_read(I2C1, MEW_ATSHA204A_I2C_ADDR, rxdata, *rxlength);
	if (st == GENERAL_ERROR_WRITE)
		return ATCA_FUNC_FAIL;
	else if (st == GENERAL_ERROR_READ)
		return ATCA_COMM_FAIL;
	else
		return ATCA_SUCCESS;
}

void change_i2c_speed(ATCAIface iface, uint32_t speed)
{
}

ATCA_STATUS hal_i2c_wake(ATCAIface iface)
{
	return ATCA_SUCCESS;;
}

ATCA_STATUS hal_i2c_idle(ATCAIface iface)
{
	return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
{
	return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_release(void *hal_data)
{
	return ATCA_SUCCESS;
}












