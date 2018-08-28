#include "display.h"
#include "../debug/debug.h"

const char * btnm_map[] = {"1", "2", "3", "\n", "4", "5", "6", "\n", "7", "8", "9", "\n", "DFU", "0", "OK", 0 };

static lv_res_t __mew_pinpad_action(lv_obj_t * btnm, const char *txt);

void mew_ui_show_pinpad(void) {
    lv_obj_t * img1 = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img1, &mew_logo);
    lv_obj_align(img1, NULL, LV_ALIGN_IN_TOP_LEFT, 6, 6);

    static lv_style_t style_txt;
    lv_style_copy(&style_txt, &lv_style_plain);
    style_txt.text.font = &lv_font_dejavu_30;
    style_txt.text.letter_space = 2;
    style_txt.text.line_space = 1;
    style_txt.text.color = LV_COLOR_HEX(0xF01010);

    lv_obj_t * label1 =  lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style(label1, &style_txt);
    lv_label_set_text(label1, "Enter PIN");
    lv_obj_set_size(label1, LV_HOR_RES - 74, 24);
    lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_LEFT, 74 + 16 + 6, 24);

    lv_obj_t * btnm1 = lv_btnm_create(lv_scr_act(), NULL);
    lv_btnm_set_map(btnm1, btnm_map);
    lv_obj_set_size(btnm1, LV_HOR_RES-16, LV_VER_RES - 64 - 6 - 8 - 8);
    lv_btnm_set_action(btnm1, __mew_pinpad_action);
    lv_obj_align(btnm1, NULL, LV_ALIGN_IN_TOP_MID, 0, 64 + 8 + 6);
}

static lv_res_t __mew_pinpad_action(lv_obj_t * btnm, const char *txt) {
    //printf("Button: %s released\n", txt);

	mew_debug_print(txt);

    return LV_RES_OK; /*Return OK because the button matrix is not deleted*/
}
