#include "../display/display.h"
#include "../debug/debug.h"
#include "ui.h"

static mew_windows _mew_windows = {0, 0, NULL};

void mew_show_window(uint32_t window_id) {
    if (window_id == 0) {
        return;
    }
    
    if (_mew_windows.current_window_id != 0) {
        lv_obj_del(_mew_windows.current_window);
        _mew_windows.last_window_id = _mew_windows.current_window_id;
    }
    
    _mew_windows.current_window_id = window_id;
    switch (window_id) {
        case MEW_WINDOW_PINPAD:
            _mew_windows.current_window = mew_ui_get_pinpad();
            break;
        case MEW_WINDOW_ROOT:
            _mew_windows.current_window = mew_ui_get_main_window();
            break;
        case MEW_WINDOW_PASSWORDS:
            _mew_windows.current_window = mew_ui_get_passwords_list_window();
            break;
        case MEW_WINDOW_PASSWORD:
            _mew_windows.current_window = mew_ui_get_password_window();
            break;
        case MEW_WINDOW_DEATH_SCREEN:
            
            break;
        default:
            _mew_windows.last_window_id = 0;
            _mew_windows.current_window_id = 0;
            _mew_windows.current_window = NULL;
            return;
    }
}

