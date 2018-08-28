#include "../debug/debug.h"
#include "./i2c.h"

void mew_i2c_init(void) {
    gpio_mode_setup(MEW_I2C_TS_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MEW_I2C_TS_SCL_PIN);
    gpio_set_output_options(MEW_I2C_TS_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, MEW_I2C_TS_SCL_PIN);
    gpio_set_af(MEW_I2C_TS_SCL_PORT, MEW_I2C_TS_SCL_AF, MEW_I2C_TS_SCL_PIN);
    
    gpio_mode_setup(MEW_I2C_TS_SDA_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MEW_I2C_TS_SDA_PIN);
    gpio_set_output_options(MEW_I2C_TS_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, MEW_I2C_TS_SDA_PIN);
    gpio_set_af(MEW_I2C_TS_SDA_PORT, MEW_I2C_TS_SDA_AF, MEW_I2C_TS_SDA_PIN);

    i2c_reset(MEW_I2C_TS_I2C);
    i2c_set_standard_mode(MEW_I2C_TS_I2C);
    i2c_set_clock_frequency(MEW_I2C_TS_I2C, I2C_CR2_FREQ_2MHZ);
    i2c_set_ccr(MEW_I2C_TS_I2C, 20);
    i2c_peripheral_enable(MEW_I2C_TS_I2C);
}

uint8_t mew_i2c_read_block_ts2007(uint8_t dev_addr, uint8_t cmd, uint8_t* data, uint8_t size) {
    uint8_t i;
    
    i2c_send_start(MEW_I2C_TS_I2C);
    while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_SB) == 0);

    i2c_send_7bit_address(MEW_I2C_TS_I2C, dev_addr, I2C_WRITE);
    while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_ADDR) == 0);

    if ((I2C_SR2(MEW_I2C_TS_I2C) & I2C_SR2_MSL) && (I2C_SR2(MEW_I2C_TS_I2C) & I2C_SR2_BUSY)) { 
        i2c_send_data(MEW_I2C_TS_I2C, cmd);
        while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_TxE) == 0);
        
        i2c_send_start(MEW_I2C_TS_I2C);
        while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_SB) == 0);

        i2c_send_7bit_address(MEW_I2C_TS_I2C, dev_addr, I2C_READ);
        while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_ADDR) == 0);
        

        if (I2C_SR2(MEW_I2C_TS_I2C) & I2C_SR2_BUSY) {
            switch (size) {
                case 0:
                    mew_die_with_message("mew_i2c_read_block_ts2007: Bad usage");
                    break;
                case 1:
                    while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_RxNE) == 0);
                    data[0] = i2c_get_data(MEW_I2C_TS_I2C);
                    
                    break;
                case 2:
                    i2c_enable_ack(MEW_I2C_TS_I2C);
                    while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_RxNE) == 0);
                    data[0] = i2c_get_data(MEW_I2C_TS_I2C);
                    
                    i2c_disable_ack(MEW_I2C_TS_I2C);
                    while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_RxNE) == 0);
                    data[1] = i2c_get_data(MEW_I2C_TS_I2C);
                    
                    break;
                default:
                    i2c_enable_ack(I2C1);
                    for (i=0; i<size; i++) {
                        if ((size - i) == 2) {
                            i2c_disable_ack(MEW_I2C_TS_I2C);
                        }
                        while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_RxNE) == 0);
                        data[i] = i2c_get_data(MEW_I2C_TS_I2C);
                    }
            }

            i2c_send_stop(MEW_I2C_TS_I2C);
            return 1;
        }
    }
    return 0;
}


uint8_t mew_i2c_read(uint8_t dev_addr, uint16_t data_addr, uint8_t* data, uint8_t mode) {
    i2c_send_start(MEW_I2C_TS_I2C);
    while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_SB) == 0);

    i2c_send_7bit_address(MEW_I2C_TS_I2C, dev_addr, I2C_WRITE);
    while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_ADDR) == 0);

    if ((I2C_SR2(MEW_I2C_TS_I2C) & I2C_SR2_MSL) && (I2C_SR2(MEW_I2C_TS_I2C) & I2C_SR2_BUSY)) { 
        if (mode == MEW_I2C_MODE_16BIT) {
            i2c_send_data(MEW_I2C_TS_I2C, ((uint8_t) (data_addr >> 8)));
            while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_TxE) == 0);
        }
        i2c_send_data(MEW_I2C_TS_I2C, ((uint8_t) data_addr));
        while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_TxE) == 0);

        i2c_send_start(MEW_I2C_TS_I2C);
        while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_SB) == 0);

        i2c_send_7bit_address(MEW_I2C_TS_I2C, dev_addr, I2C_READ);
        while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_ADDR) == 0);
        

        if (I2C_SR2(MEW_I2C_TS_I2C) & I2C_SR2_BUSY) {
            while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_RxNE) == 0);
            
            *data = i2c_get_data(MEW_I2C_TS_I2C);
            
            i2c_send_stop(MEW_I2C_TS_I2C);
            return 1;
        }
    }
    return 0;
}

uint8_t mew_i2c_write(uint8_t dev_addr, uint16_t data_addr, uint8_t data, uint8_t mode) {
    i2c_send_start(MEW_I2C_TS_I2C);
    while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_SB) == 0);

    i2c_send_7bit_address(MEW_I2C_TS_I2C, dev_addr, I2C_WRITE);
    while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_ADDR) == 0);

    if ((I2C_SR2(MEW_I2C_TS_I2C) & I2C_SR2_MSL) && (I2C_SR2(MEW_I2C_TS_I2C) & I2C_SR2_BUSY)) { 
        if (mode == MEW_I2C_MODE_16BIT) {
            i2c_send_data(MEW_I2C_TS_I2C, ((uint8_t)(data_addr >> 8)));
            while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_TxE) == 0);
        }

        i2c_send_data(MEW_I2C_TS_I2C, ((uint8_t) data_addr));
        while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_TxE) == 0);

        if (mode == MEW_I2C_MODE_8BIT || mode == MEW_I2C_MODE_16BIT) {
            i2c_send_data(MEW_I2C_TS_I2C, data);
            while ((I2C_SR1(MEW_I2C_TS_I2C) & I2C_SR1_TxE) == 0);
        }

        i2c_send_stop(MEW_I2C_TS_I2C);
        return 1;
    }
    
    return 0;
}

