#include "config.h"
#include "debug/debug.h"
#include "lv_conf.h"
#include "lvgl/lvgl.h"

//struct settings_backup_ram *backup_ram = (struct settings_backup_ram *) BKPSRAM_BASE;

volatile uint32_t systick_millis = 0;

void mew_delay_ms(uint32_t ms) {
    uint32_t expected_time = systick_millis + ms;
    while (expected_time > systick_millis) __asm__("NOP");
}

uint32_t mew_get_cpu_clock(void) {
    return rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ].ahb_frequency;
}

void mew_start_all_clock(void) {
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
}

void mew_systick_setup(void) {
    systick_set_reload(MEW_SYSTICK_RELOAD_VALUE);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    systick_interrupt_enable();
}

void sys_tick_handler(void) {
    systick_millis++;
    lv_tick_inc(1);
}

uint32_t mew_get_millis(void) {
    return systick_millis;
}

void hard_fault_handler(void) {
    mew_debug_print("hard_fault_handler");
    mew_die();
}

void usage_fault_handler(void) {
    mew_debug_print("usage_fault_handler");
    mew_die();
}

void bus_fault_handler(void) {
    mew_debug_print("bus_fault_handler");
    mew_die();
}

void mew_wait_for_state(volatile uint8_t* var, uint8_t expected_state) {
    uint32_t counter;
    for (counter=0; counter<1000000; counter++) {
        if (*var == expected_state) return;
    }
    mew_debug_print("too long wait...");
    mew_die();
}

void mew_check_dma_memory(void* pointer, char* where) {
    uint32_t p = ((uint32_t) pointer);
    //mew_debug_print_hex((uint8_t*) &pointer, 4);
    if ((p > 0x10000000) && (p < 11000000)) {
        mew_debug_print(where);
        mew_die_with_message("Illegal memory access!");
    }
}

void mew_die_with_message(char* str) {
    mew_debug_print("\n\n================= ERROR ==================");
    mew_debug_print(str);
    mew_debug_print(    "==========================================");
    mew_die();
}

void mew_die(void) {
    uint32_t counter;
    __asm__("cpsid i");
    while (1) {
        for (counter=0; counter<20000000; counter++) {
            __asm__("NOP");
        }
        mew_led_toggle();
    }
}
