#include "ui.h"
#include "../display/display.h"
#include "../debug/debug.h"

static lv_res_t __mew_pinpad_action(lv_obj_t * btnm, const char *txt);
static uint8_t __mew_get_btn_number(const char *txt);

static void __mew_add_char_to_pin(uint8_t button);
static void __mew_del_last_char_from_pin(void);

static volatile uint8_t _mew_pincode[MEW_PINCODE_MAX_SIZE];
static volatile uint8_t _mew_pincode_disp[MEW_PINCODE_MAX_SIZE];
static volatile uint32_t _mew_counter = 0;

static lv_obj_t * _mew_label_pincode;
extern const lv_img_t mew_logo;

static lv_obj_t * _mew_window;

const char * _mew_btnm_map[] = {"1", "2", "3", "\n", "4", "5", "6", "\n", "7", "8", "9", "\n", "Del", "0", "OK", 0 };

static void __mew_add_char_to_pin(uint8_t button) {
    if (_mew_counter >= MEW_PINCODE_MAX_SIZE) {
        return;
    }
    
    _mew_pincode[_mew_counter] = button + MEW_PINCODE_MAGIC_ADD;
    _mew_pincode_disp[_mew_counter] = '*';
    
    if (_mew_label_pincode != NULL) {
        lv_label_set_text(_mew_label_pincode, (char*) _mew_pincode_disp);
    }
    
    _mew_counter++;
}

static void __mew_del_last_char_from_pin(void) {
    if (_mew_counter > 0) {
        _mew_counter--;
    }
        
    _mew_pincode[_mew_counter] = 0;
    _mew_pincode_disp[_mew_counter] = ' ';
    
    if (_mew_label_pincode != NULL) {
        lv_label_set_text(_mew_label_pincode, (char*) _mew_pincode_disp);
    }
}

uint8_t*  mew_ui_get_pincode(void) {
    return (uint8_t*) _mew_pincode;
}

lv_obj_t* mew_ui_get_pinpad(void) {
    memset((void*) _mew_pincode, 0, MEW_PINCODE_MAX_SIZE);
    memset((void*) _mew_pincode_disp, 0, MEW_PINCODE_MAX_SIZE);
    
    _mew_window = lv_cont_create(lv_scr_act(), NULL);
    lv_cont_set_fit(_mew_window, true, true);
    
    lv_obj_t * img1 = lv_img_create(_mew_window, NULL);
    lv_img_set_src(img1, &mew_logo);
    lv_obj_align(img1, NULL, LV_ALIGN_IN_TOP_LEFT, 6, 6);

    static lv_style_t style_txt;
    lv_style_copy(&style_txt, &lv_style_plain);
    style_txt.text.font = &lv_font_dejavu_30;
    style_txt.text.letter_space = 2;
    style_txt.text.line_space = 1;
    style_txt.text.color = LV_COLOR_HEX(0xF01010);

    lv_obj_t * label1 =  lv_label_create(_mew_window, NULL);
    lv_obj_set_style(label1, &style_txt);
    lv_label_set_text(label1, "Enter PIN");
    lv_obj_set_size(label1, LV_HOR_RES - 74, 24);
    lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_LEFT, 74 + 32 + 6, 16);
    
    static lv_style_t style_txt2;
    lv_style_copy(&style_txt2, &lv_style_plain);
    style_txt2.text.font = &lv_font_dejavu_30;
    style_txt2.text.letter_space = 2;
    style_txt2.text.line_space = 1;
    style_txt2.text.color = LV_COLOR_HEX(0x107010);
    
    _mew_label_pincode = lv_label_create(_mew_window, NULL);
    lv_obj_set_style(_mew_label_pincode, &style_txt2);
    lv_label_set_text(_mew_label_pincode, " ");
    lv_obj_set_size(_mew_label_pincode, LV_HOR_RES - 74, 24);
    lv_obj_align(_mew_label_pincode, NULL, LV_ALIGN_IN_TOP_LEFT, 74 + 32 + 6, 44);

    lv_obj_t * btnm1 = lv_btnm_create(_mew_window, NULL);
    lv_btnm_set_map(btnm1, _mew_btnm_map);
    lv_obj_set_size(btnm1, LV_HOR_RES-16, LV_VER_RES - 64 - 6 - 8 - 8);
    lv_btnm_set_action(btnm1, __mew_pinpad_action);
    lv_obj_align(btnm1, NULL, LV_ALIGN_IN_TOP_MID, 0, 64 + 8 + 6);
    
    return _mew_window;
}

static uint8_t __mew_get_btn_number(const char *txt) {
    uint8_t i = 0;
    while (1) {
        if (*_mew_btnm_map[i] == 0) return 0;
        if (strcmp(_mew_btnm_map[i], txt) == 0) {
            return i+1;
        }
        i++;
    }
}

static lv_res_t __mew_pinpad_action(lv_obj_t * btnm, const char *txt) {
    uint8_t number = __mew_get_btn_number(txt);
    switch(number) {
        case 13: // DEL
            __mew_del_last_char_from_pin();
            break;
        case 15: // OK
            
            break;
        default:
            __mew_add_char_to_pin(number);
    }
    return LV_RES_OK;
}
