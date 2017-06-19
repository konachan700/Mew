#include "ui.h"

#define ERROR_MESSAGE_NO_SD_CARD "Can't reading root password record. Memory card is not present or corrupted."

struct menu_ui_element root_menu[MENU_ITEMS_IN_ROOT]= {{
    0,
    "Passwords", "My passwords wallet",
    icon_E32A,
    0, 0, 1, 0
}, {
    0,
    "Config mode", "Change MeW settings from PC",
    icon_E1E0,
    0, 1, 1, 0
}, /*{
    0,
    "Disk mode", "For use internal secure uSD",
    0, 0, 1, 0
},*/ {
    0,
    "About MeW", "Display information about MeW",
    icon_E02F,
    0, 0, 1, 0
}};

struct menu_ui_element password_menu[PASSWORD_MENU_ITEMS_COUNT]= {{
    0,
    "Login", " ",
    icon_E853,
    0, 0, 1, 0
}, {
    0,
    "Tab", "Send Tab key, go to next field",
    icon_E8D4,
    0, 1, 1, 0
}, {
    0,
    "Password", "Password cannot be displayed.",
    icon_E897,
    0, 0, 1, 0
}, {
    0,
    "Enter", "Send Enter key",
    icon_E5D9,
    0, 0, 1, 0
}};

extern struct settings_record mew_settings;

volatile u32 menu_type          = MENU_TYPE_MAIN;
volatile u32 current_menu_id    = 0;
s16 menu_local_sel_index        = 0;
s16 menu_local_sel_page         = 0;
s16 menu_items_count            = MENU_ITEMS_IN_ROOT;

u32 menu_id_list[MEW_PASSWORD_EXTRA_SIZE];

struct menu_ui_element passwords_displayed_menu[MENU_ELEMENTS_COUNT];
struct password_record passwords_temp_pwd_record;

void __fatal_error(u8* title, u8* text) {
    display_fill(255, 255, 255);
    direct_draw_string_ml(title, 10, 10, 200, 300, 3, 255, 0, 0);
    direct_draw_string_ml(text, 10, 40, 200, 300, 2, 0, 0, 0);
    while (1) __asm__("NOP");
}

void mem_manage_handler(void) {
    debug_print("mem_manage_handler");
    while(1) gpio_toggle(GPIOC, GPIO3);
}

void hard_fault_handler(void) {
    debug_print("hard_fault_handler");
    while(1) gpio_toggle(GPIOC, GPIO3);
}

void usage_fault_handler(void) {
    debug_print("usage_fault_handler");
    while(1) gpio_toggle(GPIOC, GPIO3);
}

void bus_fault_handler(void) {
    debug_print("bus_fault_handler");
    while(1) gpio_toggle(GPIOC, GPIO3);
}

void __message_info(u8* title) {
    __menu_paint_all();
    direct_draw_string_ml(title, STATUSBAR_W + MENU_ITEM_PADDING_LEFT, 20, 200, 300, 2, 122, 122, 122);
}

u16 __passwords_get_list_size(u32* e) {
    u16 k=0;
    for (u16 i=0; i<64; i++)
        if (e[i] != 0) k++;
    return k;
}

void __gen_passwords_menu_id_list(void) {
    if (mewcrypt_get_pwd_record(&passwords_temp_pwd_record, current_menu_id) != MEW_CRYPT_OK)
        __fatal_error("Error #90", ERROR_MESSAGE_NO_SD_CARD);
    memcpy(menu_id_list, passwords_temp_pwd_record.extra, MEW_PASSWORD_EXTRA_SIZE * sizeof(u32));
}

u32 __passwords_get_parent(u32 child_id) {
    if (child_id == 0) return MEW_PASSWORD_RECORD_NO_PARENT;
    if (mewcrypt_get_pwd_record(&passwords_temp_pwd_record, child_id) != MEW_CRYPT_OK) 
        __fatal_error("Error #92", ERROR_MESSAGE_NO_SD_CARD);
    return passwords_temp_pwd_record.parent_id;
}

void __passwords_extras_to_menu(void) {
    menu_items_count = __passwords_get_list_size(menu_id_list);
    u32 menu_id;
    
    for (u16 i=0; i<MENU_ELEMENTS_COUNT; i++) {
        menu_id = menu_id_list[i + (MENU_ELEMENTS_COUNT * menu_local_sel_page)];
        if (menu_id > 0) {
            if (mewcrypt_get_pwd_record(&passwords_temp_pwd_record, menu_id) != MEW_CRYPT_OK) 
                __fatal_error("Error #91", ERROR_MESSAGE_NO_SD_CARD);
            
            if (passwords_temp_pwd_record.magic != MEW_PASSWORD_RECORD_MAGIC)
                __fatal_error("Error #93", ERROR_MESSAGE_NO_SD_CARD);
            
            passwords_displayed_menu[i].id = passwords_temp_pwd_record.id;
            passwords_displayed_menu[i].visible = 1;
            passwords_displayed_menu[i].disp_number = i;
            passwords_displayed_menu[i].icon = ((passwords_temp_pwd_record.flags & PASSWORD_FLAG_DIRECTORY) != 0) ? icon_E2C7 : icon_E0DA;
            passwords_displayed_menu[i].selected = 0;
            passwords_displayed_menu[i].flags = passwords_temp_pwd_record.flags;

            memcpy(&passwords_displayed_menu[i].name, &passwords_temp_pwd_record.title, MEW_PASSWORD_RECORD_TITLE_LEN);
            memcpy(&passwords_displayed_menu[i].text, &passwords_temp_pwd_record.text, MEW_PASSWORD_RECORD_TEXT_LEN);
        } else {
            passwords_displayed_menu[i].id = 0;
            passwords_displayed_menu[i].visible = 0;
        }
        
        passwords_displayed_menu[menu_local_sel_index + (MENU_ELEMENTS_COUNT * menu_local_sel_page)].selected = 1;
    }
}

void __gen_one_password_menu(void) {
    if (mewcrypt_get_pwd_record(&passwords_temp_pwd_record, current_menu_id) != MEW_CRYPT_OK) 
        __fatal_error("Error #92", ERROR_MESSAGE_NO_SD_CARD);
    
    for (u16 i=0; i<MENU_ELEMENTS_COUNT; i++) {
        if (i < PASSWORD_MENU_ITEMS_COUNT) {
            memcpy(&passwords_displayed_menu[i], &password_menu[i], sizeof(struct menu_ui_element));
            passwords_displayed_menu[i].visible = 1;
            passwords_displayed_menu[i].selected = 0;
            passwords_displayed_menu[i].disp_number = i;
            passwords_displayed_menu[i].flags = 0;
        } else {
            passwords_displayed_menu[i].visible = 0;
        }
    }
    
    memcpy(passwords_displayed_menu[0].text, passwords_temp_pwd_record.login, MEW_PASSWORD_RECORD_TEXT_LEN);
    passwords_displayed_menu[0].selected = 1;
}

void __switch_menu_type(u32 type) {
    menu_local_sel_index = 0;
    menu_local_sel_page = 0;
    menu_type = type;
    switch (type) {
        case MENU_TYPE_MAIN:
            __go_to_main_menu();
            break;
        case MENU_TYPE_PASSWORDS:
            __gen_passwords_menu_id_list();
            __passwords_extras_to_menu();
            __menu_paint_all();
            break;
        case MENU_TYPE_SINGLE_PASSWORD:
            menu_items_count = PASSWORD_MENU_ITEMS_COUNT;
            __gen_one_password_menu();
            __menu_paint_all();
            break;
    }
}

void __menu_enter_hanler(u32 id, u32 type) {
    switch (type) {
        case MENU_TYPE_MAIN:
            switch (menu_local_sel_index) {
                case 0:
                    current_menu_id = 0;
                    __switch_menu_type(MENU_TYPE_PASSWORDS);
                    break;
                case 1:
                    mew_settings.global_mode = MEW_GLOBAL_MODE_CNF;
                    mewcrypt_write_settings(&mew_settings, MEW_SETTINGS_EEPROM_PAGE_OFFSET);
                    display_fill(255, 255, 255);
                    scb_reset_system();
                    while(1) __asm__("NOP");
                    break;
                case 2:
                    
                    break;
                case 3:
                    
                    break;
            }
            break;
        case MENU_TYPE_PASSWORDS:
            current_menu_id = menu_id_list[menu_local_sel_index + (MENU_ELEMENTS_COUNT * menu_local_sel_page)];
            if ((passwords_displayed_menu[menu_local_sel_index].flags & PASSWORD_FLAG_DIRECTORY) != 0) {
                __switch_menu_type(MENU_TYPE_PASSWORDS);
            } else {
                __switch_menu_type(MENU_TYPE_SINGLE_PASSWORD);
            }
            break;
        case MENU_TYPE_SINGLE_PASSWORD:
            
            
            
            __menu_down();
            break;
    }
}

void __menu_exit_hanler(u32 id, u32 type) {
    u32 cm_id;
    switch (type) {
        case MENU_TYPE_MAIN:
            
            break;
        case MENU_TYPE_PASSWORDS:
            cm_id = __passwords_get_parent(current_menu_id);
            if (cm_id == MEW_PASSWORD_RECORD_NO_PARENT) {
                __switch_menu_type(MENU_TYPE_MAIN);
            } else {
                current_menu_id = cm_id;
                __switch_menu_type(MENU_TYPE_PASSWORDS);
            }
            break;
        case MENU_TYPE_SINGLE_PASSWORD:
            cm_id = __passwords_get_parent(current_menu_id);
            current_menu_id = cm_id;
            __switch_menu_type(MENU_TYPE_PASSWORDS);
            break;
    }
}

void __go_to_main_menu(void) {
    menu_type        = MENU_TYPE_MAIN;
    current_menu_id  = 0;
    menu_items_count = MENU_ITEMS_IN_ROOT;
    menu_local_sel_index = 0;
    menu_local_sel_page = 0;
    //selected_menu_id = 0;
    
    for (u16 i=0; i<MENU_ELEMENTS_COUNT; i++) {
        if (i < MENU_ITEMS_IN_ROOT) {
            memcpy(&passwords_displayed_menu[i], &root_menu[i], sizeof(struct menu_ui_element));
            passwords_displayed_menu[i].visible = 1;
            passwords_displayed_menu[i].disp_number = i;
        } else {
            passwords_displayed_menu[i].id = 0;
            passwords_displayed_menu[i].visible = 0;
        }
        passwords_displayed_menu[i].selected = 0;
    }
    passwords_displayed_menu[0].selected = 1;
    
    __menu_paint_all();
}

void statusbar_paint(void) {
    g_clear_buf(STATUSBAR_BUF, 200, 200, 200);
    g_draw_vline(STATUSBAR_BUF, STATUSBAR_W - 1, 0, 319, 170, 170, 170);
    
    g_draw_string(STATUSBAR_BUF, "ok", 1, STATUSBAR_OK_POS, 2, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_90_DEG);
    g_draw_string(STATUSBAR_BUF, "back", 1, STATUSBAR_BACK_POS, 2, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_90_DEG);
    g_draw_string(STATUSBAR_BUF, "up", 1, STATUSBAR_UP_POS, 2, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_90_DEG);
    g_draw_string(STATUSBAR_BUF, "down", 1, STATUSBAR_DOWN_POS, 2, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_90_DEG);

    g_commit(STATUSBAR_BUF, 0);
} 

void menu_item_paint(struct menu_ui_element* element) {
    if (element->selected == 0) { 
        g_clear_buf(MENU_ELEMENT_BUF, COLOR_R_0, COLOR_G_0, COLOR_B_0);
        g_draw_hline(MENU_ELEMENT_BUF, MENU_ITEM_PADDING_LEFT, ELEMENT_W-1, ELEMENT_H-1, 150, 150, 150);
        if (element->icon != NULL)
            g_draw_icon(MENU_ELEMENT_BUF, element->icon, 2, 4, COLOR_R_1, COLOR_G_1, COLOR_B_1);
        g_draw_string(MENU_ELEMENT_BUF, element->name, MENU_ITEM_PADDING_LEFT+MEW_ICONS_FONT_WIDTH, MENU_ITEM_PADDING_TOP, MENU_ITEM_HEADER_SIZE, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_0_DEG);
        g_draw_string(MENU_ELEMENT_BUF, element->text, MENU_ITEM_PADDING_LEFT+MEW_ICONS_FONT_WIDTH, MENU_ITEM_PADDING_TOP + LINE_SPACE + (FONT_H*MENU_ITEM_HEADER_SIZE), MENU_ITEM_TEXT_SIZE, COLOR_R_1, COLOR_G_1, COLOR_B_1, FONT_0_DEG);
    } else {
        g_clear_buf(MENU_ELEMENT_BUF, 0, 150, 0);
        if (element->disp_number > 0)
            g_draw_hline(MENU_ELEMENT_BUF, 0, ELEMENT_W, 0, COLOR_R_0, COLOR_G_0, COLOR_B_0);
        if (element->icon != NULL)
            g_draw_icon(MENU_ELEMENT_BUF, element->icon, 2, 4, COLOR_R_0, COLOR_G_0, COLOR_B_0);
        g_draw_string(MENU_ELEMENT_BUF, element->name, MENU_ITEM_PADDING_LEFT+MEW_ICONS_FONT_WIDTH, MENU_ITEM_PADDING_TOP, MENU_ITEM_HEADER_SIZE, COLOR_R_0, COLOR_G_0, COLOR_B_0, FONT_0_DEG);
        g_draw_string(MENU_ELEMENT_BUF, element->text, MENU_ITEM_PADDING_LEFT+MEW_ICONS_FONT_WIDTH, MENU_ITEM_PADDING_TOP + LINE_SPACE + (FONT_H*MENU_ITEM_HEADER_SIZE), MENU_ITEM_TEXT_SIZE, COLOR_R_0, COLOR_G_0, COLOR_B_0, FONT_0_DEG);
    }
    g_commit(MENU_ELEMENT_BUF, element->disp_number);
}

void __menu_paint_all(void) {   
    for (u16 i=0; i<MENU_ELEMENTS_COUNT; i++) {
        if (menu_items_count > i) {
            if (passwords_displayed_menu[i].visible == 1) {
                passwords_displayed_menu[i].disp_number = i;
                menu_item_paint(&passwords_displayed_menu[i]);
            } else {
                g_clear_buf(MENU_ELEMENT_BUF, COLOR_R_0, COLOR_G_0, COLOR_B_0);
                g_commit(MENU_ELEMENT_BUF, i);
            }
        } else {
            g_clear_buf(MENU_ELEMENT_BUF, COLOR_R_0, COLOR_G_0, COLOR_B_0);
            g_commit(MENU_ELEMENT_BUF, i);
        }
    }
}

void __select_mi(u16 count_total) {
    for (u16 i=0; i<count_total; i++) 
        passwords_displayed_menu[i].selected = 0;
    passwords_displayed_menu[menu_local_sel_index].selected = 1;
}

void __menu_down(void) {
    if (menu_items_count == 0) return;
    
    u16 count_page = (menu_items_count > MENU_ELEMENTS_COUNT) ? MENU_ELEMENTS_COUNT : menu_items_count;
    u16 curr_index = 0;

    menu_local_sel_index++;
    if (menu_local_sel_index >= count_page) {
        menu_local_sel_index = 0;
        menu_local_sel_page++;
        
        if ((menu_local_sel_page * MENU_ELEMENTS_COUNT) >= menu_items_count) {
            menu_local_sel_page = 0;
        }
        
        //__passwords_extras_to_menu();
    }
    
    __select_mi(menu_items_count);
    __menu_paint_all();
}

void __menu_up(void) {
    if (menu_items_count == 0) return;
    
    u16 count_page = (menu_items_count > MENU_ELEMENTS_COUNT) ? MENU_ELEMENTS_COUNT : menu_items_count;
    u16 curr_index = 0;

    menu_local_sel_index--;
    if (menu_local_sel_index < 0) {
        menu_local_sel_index = (count_page - 1);
        menu_local_sel_page--;
        
        if (menu_local_sel_page < 0) {
            menu_local_sel_page = menu_items_count / MENU_ELEMENTS_COUNT;
        }
        
        //__passwords_extras_to_menu();
    }

    __select_mi(menu_items_count);
    __menu_paint_all();
}

void menu_init(void) {
    menu_local_sel_index = 0;
    menu_local_sel_page = 0;
    
    __go_to_main_menu();
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
            __menu_enter_hanler(current_menu_id, menu_type);
            /*if (selected_menu != NULL)
                if (selected_menu->on_enter != NULL)
                    selected_menu->on_enter(selected_menu_id, menu_type);*/
            break;
        case BUTTON_BACK:
            __menu_exit_hanler(current_menu_id, menu_type);
            break;
    }
}





























