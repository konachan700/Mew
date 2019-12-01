#include "system.h"

unsigned int mew_clock_init(void) {
    rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
    
    //pwr_disable_backup_domain_write_protect();
    
    rcc_periph_clock_enable(RCC_SYSCFG);
    //rcc_periph_clock_enable(RCC_RTC);
    //rcc_periph_clock_enable(RCC_BKPSRAM);
    
    //rcc_periph_clock_enable(RCC_ADC1);
    
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_GPIOE);
    
    rcc_periph_clock_enable(RCC_DMA1);
    rcc_periph_clock_enable(RCC_DMA2);
    
    //rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_USART2);
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_SPI2);
    rcc_periph_clock_enable(RCC_OTGFS);
    rcc_periph_clock_enable(RCC_I2C3);
    rcc_periph_clock_enable(RCC_RNG);
    rcc_periph_clock_enable(RCC_CRYP);
    rcc_periph_clock_enable(RCC_CRC);
    rcc_periph_clock_enable(RCC_HASH);
    
    return 0;
}
