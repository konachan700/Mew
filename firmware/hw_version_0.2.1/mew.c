#include "config.h"
#include "debug/debug.h"
#include "display/display.h"
#include "usb/mew_usb_hid.h"
#include "flash/flash.h"
#include "bluetooth/bluetooth.h"
#include "crypto/crypto.h"
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "i2c/i2c.h"
#include "ui/ui.h"

int main(void) {
    //uint32_t i;
    
    mew_start_all_clock();
    mew_hid_usb_init();
    mew_systick_setup();
   
    mew_led_init();
    mew_start_debug_usart();
    mew_start_random();
    mew_ss_init();
    
    mew_i2c_init();
    mew_spi_flash_init();
    mew_bluetooth_init();
    mew_bluetooth_config();
    
    mew_display_init();
    //mew_touchscreen_init();
    
    //lv_obj_t * label1 =  lv_label_create(lv_scr_act(), NULL);
    //lv_label_set_text(label1, "Enter PIN");
    //lv_obj_set_size(label1, LV_HOR_RES - 74, 24);
    //lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    mew_ui_show_pinpad();
//mew_debug_print("\n--- Started ---");
    
    mew_touchscreen_init();
    //
    

    //
    
    mew_integration_test();
    
    while(1) {
        mew_delay_ms(15);
        //for (i=0; i<100000; i++) __asm__("NOP");
        
    	//if (mew_is_spi_dma_bisy() == 0) {
            lv_task_handler();
    	//}


        
    }
    return 1;
}
