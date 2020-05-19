#include "ui.h"
#include "lv_conf.h"
#include "lvgl/lvgl.h"

extern lv_font_t mew_font_ru_bold;

static lv_style_t _mew_msgbox_shadow_style_1, _mew_style_txt;
static lv_obj_t * _mew_cont3;
static lv_obj_t * _mew_mbox1;

static const char * _mew_msgbox_btns[] ={"Close", ""};

static lv_event_cb_t mew_event_cb_t;

static void _mew_msgbox_event_handler(lv_obj_t * obj, lv_event_t event) {
    if(event == LV_EVENT_VALUE_CHANGED) {
    	lv_obj_del(_mew_cont3);
    	lv_task_handler();
    	mew_event_cb_t(obj, event);
    }
}

void mew_ui_msgbox(const char * text, lv_event_cb_t evt) {
    _mew_cont3 = lv_cont_create(lv_scr_act(), NULL);
    lv_cont_set_layout(_mew_cont3, LV_LAYOUT_OFF);
    lv_obj_align(_mew_cont3, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_set_size(_mew_cont3, LV_HOR_RES, LV_VER_RES);

    lv_style_copy(&_mew_msgbox_shadow_style_1, &lv_style_plain);
    _mew_msgbox_shadow_style_1.body.main_color = LV_COLOR_MAKE(0, 0, 0);
    _mew_msgbox_shadow_style_1.body.grad_color = LV_COLOR_MAKE(0, 0, 0);
    _mew_msgbox_shadow_style_1.body.opa = 127;
    _mew_msgbox_shadow_style_1.text.font = &mew_font_ru_bold;
    lv_cont_set_style(_mew_cont3, LV_CONT_STYLE_MAIN, &_mew_msgbox_shadow_style_1);

    lv_style_copy(&_mew_style_txt, &lv_style_pretty);
    _mew_style_txt.body.padding.top= 8;
    _mew_style_txt.body.padding.inner = 24;
    _mew_style_txt.text.font = &mew_font_ru_bold;
    _mew_style_txt.text.letter_space = 0;
    _mew_style_txt.text.line_space = 0;
    _mew_style_txt.text.color = LV_COLOR_MAKE(0, 0, 0);

	_mew_mbox1 = lv_mbox_create(_mew_cont3, NULL);
    lv_mbox_set_text(_mew_mbox1, text);
    lv_mbox_add_btns(_mew_mbox1, _mew_msgbox_btns);
    lv_obj_set_width(_mew_mbox1, 200);
    lv_obj_set_event_cb(_mew_mbox1, _mew_msgbox_event_handler);
    lv_obj_align(_mew_mbox1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_btnm_set_style(_mew_mbox1, LV_BTNM_STYLE_BG, &_mew_style_txt);
    //lv_btnm_set_style(_mew_mbox1, LV_BTNM_STYLE_BTN_REL, &_mew_style_txt);

    mew_event_cb_t = evt;
}
