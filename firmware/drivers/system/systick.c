#include "system.h"

#include "lv_conf.h"
#include "lvgl/lvgl.h"

static volatile unsigned int systick_millis = 0;

unsigned int mew_systick_init(void) {
    systick_set_reload(MEW_SYSTICK_RELOAD_VALUE);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    systick_interrupt_enable();
    
    return 0;
}

unsigned int mew_get_millis(void) {
    return systick_millis;
}

void mew_delay_ms(uint32_t ms) {
    uint32_t expected_time = systick_millis + ms;
    while (expected_time > systick_millis) __asm__("NOP");
}

void sys_tick_handler(void) {
    systick_millis++;
    lv_tick_inc(1);
}
