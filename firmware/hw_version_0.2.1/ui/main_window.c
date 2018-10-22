#include "ui.h"
#include "../display/display.h"
#include "../debug/debug.h"

static lv_obj_t * _mew_window;

lv_obj_t* mew_ui_get_main_window(void) {
    _mew_window = lv_cont_create(lv_scr_act(), NULL);
    lv_cont_set_fit(_mew_window, true, true);
    
    
    
    
    return _mew_window;
}