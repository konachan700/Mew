#include "ui.h"
#include "lv_conf.h"
#include "lvgl/lvgl.h"

extern const lv_img_dsc_t mew_logo;
extern lv_font_t mew_font_ru_bold_title;

static lv_obj_t * _mew_cont = NULL;
static lv_style_t _mew_style_txt, _mew_style_txt2, _mew_cont_style;
static lv_obj_t * _mew_img1;
static lv_obj_t * _mew_label1;
static lv_obj_t * _mew_label2;
static lv_obj_t * _mew_label_pincode;

void mew_ui_header_set_text(char* text) {
	if (_mew_cont == NULL) return;
	lv_label_set_text(_mew_label_pincode, (const char *) text);
}

void mew_display_small_header(void) {
	if (_mew_cont != NULL) return;

    _mew_cont = lv_cont_create(lv_scr_act(), NULL);
    lv_cont_set_layout(_mew_cont, LV_LAYOUT_OFF);
    lv_obj_align(_mew_cont, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_set_size(_mew_cont, LV_HOR_RES, 30);

    lv_style_copy(&_mew_cont_style, &lv_style_plain);
    _mew_cont_style.body.main_color = LV_COLOR_MAKE(230, 230, 230);
    _mew_cont_style.body.grad_color = LV_COLOR_MAKE(230, 230, 230);
    _mew_cont_style.body.border.color = LV_COLOR_MAKE(200, 200, 200);
    lv_cont_set_style(_mew_cont, LV_CONT_STYLE_MAIN, &_mew_cont_style);

    lv_style_copy(&_mew_style_txt, &lv_style_plain);
    _mew_style_txt.text.font = &mew_font_ru_bold_title;
    _mew_style_txt.text.letter_space = 0;
    _mew_style_txt.text.line_space = 1;
    _mew_style_txt.text.color = LV_COLOR_MAKE(0xFF, 0x00, 0x00);

    _mew_label1 = lv_label_create(_mew_cont, NULL);
    lv_obj_set_style(_mew_label1, &_mew_style_txt);
    lv_label_set_text(_mew_label1, "MeW HPM");
    lv_obj_set_size(_mew_label1, LV_HOR_RES, 24);
    lv_obj_align(_mew_label1, NULL, LV_ALIGN_IN_TOP_LEFT, 8, 4);

    _mew_label2 = lv_label_create(_mew_cont, NULL);
    lv_obj_set_style(_mew_label2, &_mew_style_txt);
    lv_label_set_text(_mew_label2, "99090");
    lv_obj_set_size(_mew_label2, LV_HOR_RES, 24);
    lv_obj_align(_mew_label2, NULL, LV_ALIGN_IN_TOP_RIGHT, -8, 4);
}

void mew_remove_ui_header(void) {
	lv_obj_del(_mew_cont);
	_mew_cont = NULL;
}

void mew_display_ui_header(char* text) {
	if (_mew_cont != NULL) return;

    _mew_cont = lv_cont_create(lv_scr_act(), NULL);
    lv_cont_set_layout(_mew_cont, LV_LAYOUT_OFF);
    lv_obj_align(_mew_cont, NULL, LV_ALIGN_IN_TOP_LEFT, 3, 3);
    lv_obj_set_size(_mew_cont, LV_HOR_RES-6, 80);

    lv_style_copy(&_mew_cont_style, &lv_style_transp);
    lv_cont_set_style(_mew_cont, LV_CONT_STYLE_MAIN, &lv_style_scr);

    _mew_img1 = lv_img_create(_mew_cont, NULL);
    lv_img_set_src(_mew_img1, &mew_logo);
    lv_obj_align(_mew_img1, NULL, LV_ALIGN_IN_TOP_LEFT, 6, 6);

    lv_style_copy(&_mew_style_txt, &lv_style_plain);
    _mew_style_txt.text.font = &mew_font_ru_bold_title;
    _mew_style_txt.text.letter_space = 0;
    _mew_style_txt.text.line_space = 1;
    _mew_style_txt.text.color = LV_COLOR_MAKE(0xF0, 0x10, 0x10);

    _mew_label1 = lv_label_create(_mew_cont, NULL);
    lv_obj_set_style(_mew_label1, &_mew_style_txt);
    lv_label_set_text(_mew_label1, "MeW HPM");
    lv_obj_set_size(_mew_label1, LV_HOR_RES - 74, 24);
    lv_obj_align(_mew_label1, NULL, LV_ALIGN_IN_TOP_LEFT, 74 + 16 + 6, 14);

    lv_style_copy(&_mew_style_txt2, &lv_style_plain);
    _mew_style_txt2.text.font = &mew_font_ru_bold_title;
    _mew_style_txt2.text.letter_space = 0;
    _mew_style_txt2.text.line_space = 1;
    _mew_style_txt2.text.color = LV_COLOR_MAKE(0x00, 0x00, 0x00);

    _mew_label_pincode = lv_label_create(_mew_cont, NULL);
    lv_obj_set_style(_mew_label_pincode, &_mew_style_txt2);
    lv_label_set_text(_mew_label_pincode, text);
    lv_obj_set_size(_mew_label_pincode, LV_HOR_RES - 74, 24);
    lv_obj_align(_mew_label_pincode, NULL, LV_ALIGN_IN_TOP_LEFT, 74 + 16 + 6, 44);
}


