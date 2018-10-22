#ifndef _MEW_UI_H
#define _MEW_UI_H

#include "../config.h"

#define MEW_PINCODE_MAX_SIZE 8
#define MEW_PINCODE_MAGIC_ADD 0x43

#define MEW_WINDOW_PINPAD       1
#define MEW_WINDOW_ROOT         2
#define MEW_WINDOW_PASSWORDS    3
#define MEW_WINDOW_PASSWORD     4

#define MEW_WINDOW_DEATH_SCREEN 99

typedef struct {
    uint32_t last_window_id;
    uint32_t current_window_id;
    lv_obj_t *current_window;
} mew_windows;

void mew_ui_show_pinpad(void);

lv_obj_t* mew_ui_get_pinpad(void);
lv_obj_t* mew_ui_get_main_window(void);
lv_obj_t* mew_ui_get_passwords_list_window(void);
lv_obj_t* mew_ui_get_password_window(void);

#endif /* UI_H */

