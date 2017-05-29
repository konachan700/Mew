#include "ui.h"

struct menu_ui_element menu[MENU_ELEMENTS_COUNT];
struct menu_ui_element* current_menu_page = NULL;
struct menu_ui_element* selected_menu = NULL;
signed int menu_local_sel_index = 0;

void statusbar_paint(void) {
    g_clear_buf(STATUSBAR_BUF, COLOR_R_0, COLOR_G_0, COLOR_B_0);
    g_draw_vline(STATUSBAR_BUF, STATUSBAR_W - 1, 0, 319, COLOR_R_1, COLOR_G_1, COLOR_B_1);
    
    g_draw_string(STATUSBAR_BUF, "ok", 1, STATUSBAR_OK_POS, 2, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_90_DEG);
    g_draw_string(STATUSBAR_BUF, "back", 1, STATUSBAR_BACK_POS, 2, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_90_DEG);
    g_draw_string(STATUSBAR_BUF, "up", 1, STATUSBAR_UP_POS, 2, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_90_DEG);
    g_draw_string(STATUSBAR_BUF, "down", 1, STATUSBAR_DOWN_POS, 2, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_90_DEG);

    g_commit(STATUSBAR_BUF, 0);
} 

void menu_item_paint(struct menu_ui_element* element) {
    if (element->selected == 0) { 
        g_clear_buf(MENU_ELEMENT_BUF, COLOR_R_0, COLOR_G_0, COLOR_B_0);
        g_draw_hline(MENU_ELEMENT_BUF, MENU_ITEM_PADDING_LEFT, ELEMENT_W-1, ELEMENT_H-1, COLOR_R_1, COLOR_G_1, COLOR_B_1);
        g_draw_string(MENU_ELEMENT_BUF, element->name, MENU_ITEM_PADDING_LEFT, MENU_ITEM_PADDING_TOP, MENU_ITEM_HEADER_SIZE, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_0_DEG);
        g_draw_string(MENU_ELEMENT_BUF, element->text, MENU_ITEM_PADDING_LEFT, MENU_ITEM_PADDING_TOP + LINE_SPACE + (FONT_H*MENU_ITEM_HEADER_SIZE), MENU_ITEM_TEXT_SIZE, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_0_DEG);
    } else {
        g_clear_buf(MENU_ELEMENT_BUF, COLOR_R_1, COLOR_G_1, COLOR_B_1);
        g_draw_hline(MENU_ELEMENT_BUF, 0, ELEMENT_W, 0, COLOR_R_0, COLOR_G_0, COLOR_B_0);
        g_draw_string(MENU_ELEMENT_BUF, element->name, MENU_ITEM_PADDING_LEFT, MENU_ITEM_PADDING_TOP, MENU_ITEM_HEADER_SIZE, COLOR_R_0, COLOR_G_0, COLOR_B_0, FONT_0_DEG);
        g_draw_string(MENU_ELEMENT_BUF, element->text, MENU_ITEM_PADDING_LEFT, MENU_ITEM_PADDING_TOP + LINE_SPACE + (FONT_H*MENU_ITEM_HEADER_SIZE), MENU_ITEM_TEXT_SIZE, COLOR_R_0, COLOR_G_0, COLOR_B_0, FONT_0_DEG);
    }
    g_commit(MENU_ELEMENT_BUF, element->disp_number);
}

void menu_reset_selection(void) {
    u16 i;
    for (i=0; i<MENU_ELEMENTS_COUNT; i++) menu[i].selected = 0;
    if (MENU_ELEMENTS_COUNT > 0) {
        menu[0].selected = 1;
        selected_menu = &menu[0];
    }
}

void menu_add_all(struct menu_ui_element* element, u16 count) {
    u16 i;
    for (i=0; i<count; i++) {
        if (i >= MENU_ELEMENTS_COUNT) return;
        menu[i] = element[i];
    }
}

void menu_paint_all(void) {
    u16 i;
    for (i=0; i<MENU_ELEMENTS_COUNT; i++) {
        if (menu[i].visible == 1) {
            menu[i].disp_number = i;
            menu_item_paint(&menu[i]);
        } else {
            g_clear_buf(MENU_ELEMENT_BUF, COLOR_R_0, COLOR_G_0, COLOR_B_0);
            g_commit(MENU_ELEMENT_BUF, i);
        }
    }
}

u16 __get_menu_count(void) {
    u16 i;
    for (i=0; i<MENU_ELEMENTS_COUNT; i++) 
        if (menu[i].visible == 0) return i;
    return MENU_ELEMENTS_COUNT;
}

void __menu_down(void) {
    u16 count = __get_menu_count();
    menu_local_sel_index++;
    if (menu_local_sel_index >= count) {
        menu_local_sel_index = 0;
        menu[0].selected = 1;
        menu[count - 1].selected = 0;
        menu_item_paint(&menu[0]);
        menu_item_paint(&menu[count - 1]);
        selected_menu = &menu[0];
    } else {
        menu[menu_local_sel_index].selected = 1;
        menu[menu_local_sel_index - 1].selected = 0;
        menu_item_paint(&menu[menu_local_sel_index]);
        menu_item_paint(&menu[menu_local_sel_index - 1]);
        selected_menu = &menu[menu_local_sel_index];
    }
}

void __menu_up(void) {
    menu_local_sel_index--;
    if (menu_local_sel_index < 0) {
        u16 count = __get_menu_count();
        menu_local_sel_index = count - 1;
        menu[0].selected = 0;
        menu[count - 1].selected = 1;
        menu_item_paint(&menu[0]);
        menu_item_paint(&menu[count - 1]);
        selected_menu = &menu[count - 1];
    } else {
        menu[menu_local_sel_index].selected = 1;
        menu[menu_local_sel_index + 1].selected = 0;
        menu_item_paint(&menu[menu_local_sel_index]);
        menu_item_paint(&menu[menu_local_sel_index + 1]);
        selected_menu = &menu[menu_local_sel_index];
    }
}

void button_pressed(u8 button) {
    switch (button) {
        case BUTTON_UP:
            __menu_up();
            break;
        case BUTTON_DOWN:
            __menu_down();
            break;
        case BUTTON_OK:
            if (selected_menu != NULL)
                if (selected_menu->on_enter != NULL)
                    selected_menu->on_enter(selected_menu);
            break;
        case BUTTON_BACK:
            if (current_menu_page != NULL)
                if (current_menu_page->on_leave != NULL)
                    current_menu_page->on_leave(current_menu_page);
            break;
    }
}





























