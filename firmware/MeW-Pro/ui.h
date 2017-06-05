#ifndef _MEW_UI_
#define _MEW_UI_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ILI9341.h"
#include "font_icons_gmd.h"

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
    void (*on_enter)(struct menu_ui_window* this_window);
    void (*on_leave)(struct menu_ui_window* this_window);
};

struct menu_ui_element {
    u32 id;
    u8* name;
    u8* text;
    const u8* icon;
    u8 disp_number;
    u8 selected;
    u8 visible;
    void (*on_enter)(struct menu_ui_element* this_menu);
    void (*on_leave)(struct menu_ui_element* this_menu);
    struct menu_ui_element* parent;
};

extern void button_pressed(u8 button);
extern void statusbar_paint(void);
extern void menu_paint_all(void);
extern void menu_reset_selection(void);
extern void menu_add_all(struct menu_ui_element* element, u16 count);

#endif
