#ifndef _MEW_UI_
#define _MEW_UI_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ILI9341.h"
#include "font_icons_gmd.h"
#include "crypt.h"
#include "usb_hid.h"

#include <libopencm3/stm32/gpio.h>

#ifndef LIGHT_UI_THEME
    #define COLOR_R_1 255
    #define COLOR_G_1 255
    #define COLOR_B_1 0

    #define COLOR_R_0 0
    #define COLOR_G_0 0
    #define COLOR_B_0 0
#else
    #define COLOR_R_1 0
    #define COLOR_G_1 0
    #define COLOR_B_1 0

    #define COLOR_R_0 255
    #define COLOR_G_0 255
    #define COLOR_B_0 255
#endif

#define MENU_TYPE_MAIN 1
#define MENU_TYPE_PASSWORDS 2

#define MENU_ITEMS_IN_ROOT          3

#define MENU_ITEM_PADDING_TOP       4
#define MENU_ITEM_PADDING_LEFT      7
#define MENU_ITEM_HEADER_SIZE       2
#define MENU_ITEM_TEXT_SIZE         1

#define STATUSBAR_DOWN_POS          87
#define STATUSBAR_BACK_POS          270
#define STATUSBAR_OK_POS            192
#define STATUSBAR_UP_POS            1

#define BUTTON_NONE     0
#define BUTTON_UP       1 
#define BUTTON_DOWN     2 
#define BUTTON_OK       3 
#define BUTTON_BACK     4 

#define MEW_BUTTON_UP_PRESSED         ((gpio_port_read(GPIOE) & GPIO15) == 0)
#define MEW_BUTTON_DOWN_PRESSED       ((gpio_port_read(GPIOE) & GPIO13) == 0)
#define MEW_BUTTON_OK_PRESSED         ((gpio_port_read(GPIOE) & GPIO12) == 0)
#define MEW_BUTTON_BACK_PRESSED       ((gpio_port_read(GPIOE) & GPIO14) == 0)

struct menu_ui_window {
    u8* title;
    u8* text;
    u8 type;
    void (*on_enter)(u32 id, u32 type);
    void (*on_leave)(u32 id, u32 type);
};

struct menu_ui_element { //119
    u32 id;
    u8 name[MEW_PASSWORD_RECORD_TITLE_LEN];
    u8 text[MEW_PASSWORD_RECORD_TEXT_LEN];
    const u8* icon;
    u8 disp_number;
    u8 selected;
    u8 visible;
    void (*on_enter)(u32 id, u32 type);
    void (*on_leave)(u32 id, u32 type);
    struct menu_ui_element* parent;
};

u16 __passwords_get_list_size(u32* e);
void __gen_passwords_menu_id_list(void);
u32 __passwords_get_parent(u32 child_id);
void __passwords_extras_to_menu(void);
void __go_to_main_menu(void);
void __menu_paint_all(void);
void __select_mi(u16 count_total);

void __menu_enter_hanler(u32 id, u32 type);
void __menu_exit_hanler(u32 id, u32 type);

extern void button_pressed(u8 button);
extern void statusbar_paint(void);
extern void menu_init(void);

#endif
