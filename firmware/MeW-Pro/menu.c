#include "menu.h"

void __passwords_menu_enter(struct menu_ui_element* this_menu) {
    
}

void __about_menu_enter(struct menu_ui_element* this_menu) {
    gpio_toggle(GPIOC, GPIO3);
}



struct menu_ui_element root_menu[4]= {{
    0,
    "Passwords",
    "My passwords wallet (HID-KB)",
    0,
    0,
    1,
    __passwords_menu_enter,
    NULL,
    NULL
}, {
    0,
    "Config mode",
    "For change MeW settings (CDC ACM)",
    0,
    1,
    1,
    NULL,
    NULL,
    NULL
}, {
    0,
    "Disk mode",
    "For use internal secure uSD (MSD)",
    0,
    0,
    1,
    NULL,
    NULL,
    NULL
}, {
    0,
    "About MeW",
    "Display information about MeW",
    0,
    0,
    1,
    __about_menu_enter,
    NULL,
    NULL
}};

void draw_root_menu(void) {
    menu_add_all(root_menu, 4);
    menu_reset_selection();
    menu_paint_all();
}















