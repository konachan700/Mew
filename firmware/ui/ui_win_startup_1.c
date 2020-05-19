#include "ui.h"
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "debug.h"
#include "drivers/hw_crypt/crypto.h"
#include "drivers/flash/flash.h"

extern lv_font_t mew_font_ru_bold;

#define MEW_PINCODE_TEXT_1 	"Create the PIN-code and\npress OK button."
#define MEW_PINCODE_TEXT_2 	"Repeat the PIN-code and\npress OK button."

static void _mew_btnm_event_handler(lv_obj_t * obj, lv_event_t event);
static void _mew_btnm2_event_handler(lv_obj_t * obj, lv_event_t event);
static void _mew_event_handler_1(lv_obj_t * btn, lv_event_t event);
static void _mew_event_handler_2(lv_obj_t * btn, lv_event_t event);
static void _mew_pc_disp(void);
static void _mew_msgbox_bad_pin_event_handler(lv_obj_t * obj, lv_event_t event);
static void _mew_msgbox_event_handler(lv_obj_t * obj, lv_event_t event);

static lv_obj_t * _mew_cont2 = NULL;
static lv_obj_t * _mew_label1;
static lv_obj_t * _mew_btn1, *_mew_btn2;
static lv_obj_t * _mew_btn1_label, *_mew_btn2_label;
static lv_obj_t * _mew_btnm1;

static lv_style_t _mew_style_txt, _mew_btn_style_1, _mew_btn_style_2, _mew_cont_style;

static const char * _mew_btnm_map[] = {"1", "2", "3", "\n", "4", "5", "6", "\n", "7", "8", "9", "\n", "Del", "0", "OK", 0 };

static uint8_t _mew_pinLength = 0, _mew_pinIndex = 0;
static uint8_t _mew_pincode[2][MEW_PINCODE_LEN];
static char _mew_pincode_box[8];
static unsigned char _mew_pincode_sha[8];

extern mew_secure_settings mew_ss;

static void _mew_msgbox_event_handler(lv_obj_t * obj, lv_event_t event) {
	_mew_pinLength = 0;
	memset(_mew_pincode[0], 0, MEW_PINCODE_LEN);
	memset(_mew_pincode[1], 0, MEW_PINCODE_LEN);
	_mew_pinIndex = 0;
	lv_label_set_text(_mew_label1, MEW_PINCODE_TEXT_1);
	_mew_pc_disp();
}

static void _mew_msgbox_bad_pin_event_handler(lv_obj_t * obj, lv_event_t event) {
	_mew_pinLength = 0;
	memset(_mew_pincode[0], 0, MEW_PINCODE_LEN);
	_mew_pc_disp();
}

static void _mew_pc_disp(void) {
	uint8_t sym_count = (_mew_pinLength % 8), i;
	for (i=0; i<8; i++) {
		if (i < sym_count) {
			_mew_pincode_box[i] = '*';
		} else {
			_mew_pincode_box[i] = 0;
		}
	}
	mew_ui_header_set_text(_mew_pincode_box);
}

static void _mew_btnm2_event_handler(lv_obj_t * obj, lv_event_t event) {
	if (event == LV_EVENT_VALUE_CHANGED) {
	    uint16_t index = lv_btnm_get_pressed_btn(obj);
	    if (index == 9) { // DELETE
			_mew_pinLength = 0;
			memset(_mew_pincode[0], 0, MEW_PINCODE_LEN);
			_mew_pc_disp();
	    }

	    if (index == 11) {
	    	mew_hash8(_mew_pincode[0], MEW_PINCODE_LEN, _mew_pincode_sha);
	    	if (memcmp(_mew_pincode_sha, mew_ss.pincode_sha, 8) == 0) {
	    		lv_obj_del(_mew_cont2);
	    		mew_remove_ui_header();
	    		mew_ui_show_window(MEW_WINDOW_MAIN_WIN);
	    	} else {
	    		mew_ui_msgbox("Неправильный PIN-код.", _mew_msgbox_bad_pin_event_handler);
	    	}
	    }

    	if (_mew_pinLength < MEW_PINCODE_LEN) {
    		_mew_pincode[0][_mew_pinLength] = (uint8_t) index & 0xFF;
    		_mew_pinLength++;
    		_mew_pc_disp();
    	}
	}
}

static void _mew_btnm_event_handler(lv_obj_t * obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
    	uint16_t index = lv_btnm_get_pressed_btn(obj);
    	if (index == 9) { // DELETE
    		if (_mew_pinLength > 0) {
    			_mew_pinLength = 0;
    			memset(_mew_pincode[_mew_pinIndex], 0, MEW_PINCODE_LEN);
    			_mew_pc_disp();
    		} else {
    			if (_mew_pinIndex == 1) {
    				_mew_pinIndex = 0;
    				_mew_pinLength = 0;
    				lv_label_set_text(_mew_label1, MEW_PINCODE_TEXT_1);
    				_mew_pc_disp();
    			}
    		}
    		return;
    	}

    	if (index == 11) {
    		if (_mew_pinIndex == 1) {
    			if (memcmp(_mew_pincode[0], _mew_pincode[1], MEW_PINCODE_LEN) == 0) {
    				mew_hash8(_mew_pincode[1], MEW_PINCODE_LEN, _mew_pincode_sha);

    				memcpy(mew_ss.pincode_sha, _mew_pincode_sha, 8);
    				mew_otp_write_temporary_data();
    				mew_save_secure_settings();

    	    		lv_obj_del(_mew_cont2);
    	    		mew_remove_ui_header();
    	    		mew_ui_show_window(MEW_WINDOW_MAIN_WIN);
    			} else {
    				mew_ui_msgbox("Pincodes are not equals!", _mew_msgbox_event_handler);
    			}
    		} else {
    			_mew_pinIndex = 1;
    			_mew_pinLength = 0;
    			memset(_mew_pincode[_mew_pinIndex], 0, MEW_PINCODE_LEN);
    			lv_label_set_text(_mew_label1, MEW_PINCODE_TEXT_2);
    			_mew_pc_disp();
    		}
			return;
		}

    	if (_mew_pinLength < MEW_PINCODE_LEN) {
    		_mew_pincode[_mew_pinIndex][_mew_pinLength] = (uint8_t) index & 0xFF;
    		_mew_pinLength++;
    		_mew_pc_disp();
    	}
    }
}

// QR-code button action
static void _mew_event_handler_1(lv_obj_t * btn, lv_event_t event) {
	if (event == LV_EVENT_CLICKED) {
		if (_mew_cont2 == NULL) return;
		lv_obj_del(_mew_cont2);
		lv_task_handler();
		_mew_cont2 = NULL;
		mew_ui_show_window(MEW_WINDOW_INITIAL_QR);
	}
}

// "Later" button action
static void _mew_event_handler_2(lv_obj_t * btn, lv_event_t event) {
	if (event == LV_EVENT_CLICKED) {
		lv_obj_clean(_mew_cont2);

	    lv_style_copy(&_mew_style_txt, &lv_style_plain);
	    _mew_style_txt.text.font = &lv_font_roboto_12;
	    _mew_style_txt.text.letter_space = 0;
	    _mew_style_txt.text.line_space = 0;
	    _mew_style_txt.text.color = LV_COLOR_MAKE(0, 0, 0);

	    _mew_label1 = lv_label_create(_mew_cont2, NULL);
	    lv_obj_set_style(_mew_label1, &_mew_style_txt);
	    lv_label_set_text(_mew_label1, MEW_PINCODE_TEXT_1);
	    lv_obj_align(_mew_label1, NULL, LV_ALIGN_IN_TOP_LEFT, 6, 0);
	    lv_obj_set_size(_mew_label1, LV_HOR_RES-12-6, 35);

		_mew_btnm1 = lv_btnm_create(_mew_cont2, NULL);
		lv_btnm_set_map(_mew_btnm1, _mew_btnm_map);
		lv_obj_align(_mew_btnm1, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 35);
		lv_obj_set_size(_mew_btnm1, LV_HOR_RES-8-6, 190);
		lv_obj_set_event_cb(_mew_btnm1, _mew_btnm_event_handler);
	}
}

void mew_show_startup_window_1(void) {
	if (mew_ss.is_valid == 1) {
		mew_display_ui_header(" ");

		_mew_cont2 = lv_cont_create(lv_scr_act(), NULL);
		lv_cont_set_layout(_mew_cont2, LV_LAYOUT_OFF);
		lv_obj_align(_mew_cont2, NULL, LV_ALIGN_IN_TOP_LEFT, 3, 89);
		lv_obj_set_size(_mew_cont2, LV_HOR_RES-6, LV_VER_RES-92);

		lv_style_copy(&_mew_cont_style, &lv_style_transp);
		lv_cont_set_style(_mew_cont2, LV_CONT_STYLE_MAIN, &lv_style_scr);

		lv_style_copy(&_mew_style_txt, &lv_style_plain);
		_mew_style_txt.text.font = &mew_font_ru_bold;
		_mew_style_txt.text.letter_space = 0;
		_mew_style_txt.text.line_space = 0;
		_mew_style_txt.text.color = LV_COLOR_MAKE(0, 0, 0);

		_mew_label1 = lv_label_create(_mew_cont2, NULL);
		lv_obj_set_style(_mew_label1, &_mew_style_txt);
		lv_label_set_long_mode(_mew_label1, LV_LABEL_LONG_BREAK);
		lv_obj_set_size(_mew_label1, LV_HOR_RES-24, 100);
		lv_label_set_text(_mew_label1, "Введите PIN-код");
		lv_obj_align(_mew_label1, NULL, LV_ALIGN_IN_TOP_LEFT, 12, 0);

		_mew_btnm1 = lv_btnm_create(_mew_cont2, NULL);
		lv_btnm_set_map(_mew_btnm1, _mew_btnm_map);
		lv_obj_align(_mew_btnm1, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 35);
		lv_obj_set_size(_mew_btnm1, LV_HOR_RES-8-6, 190);
		lv_obj_set_event_cb(_mew_btnm1, _mew_btnm2_event_handler);
	} else {
		mew_display_ui_header("v0.2.2 Dev");

		_mew_cont2 = lv_cont_create(lv_scr_act(), NULL);
		lv_cont_set_layout(_mew_cont2, LV_LAYOUT_OFF);
		lv_obj_align(_mew_cont2, NULL, LV_ALIGN_IN_TOP_LEFT, 3, 89);
		lv_obj_set_size(_mew_cont2, LV_HOR_RES-6, LV_VER_RES-92);

		lv_style_copy(&_mew_cont_style, &lv_style_transp);
		lv_cont_set_style(_mew_cont2, LV_CONT_STYLE_MAIN, &lv_style_scr);

		lv_style_copy(&_mew_style_txt, &lv_style_plain);
		_mew_style_txt.text.font = &mew_font_ru_bold;
		_mew_style_txt.text.letter_space = 0;
		_mew_style_txt.text.line_space = 0;
		_mew_style_txt.text.color = LV_COLOR_MAKE(0, 0, 0);

		_mew_label1 = lv_label_create(_mew_cont2, NULL);
		lv_obj_set_style(_mew_label1, &_mew_style_txt);
		lv_label_set_long_mode(_mew_label1, LV_LABEL_LONG_BREAK);
		lv_obj_set_size(_mew_label1, LV_HOR_RES-24, 100);
		lv_label_set_text(_mew_label1, "Это первый запуск MeW.\nДля быстрого подключения к смартфону нажмите кнопку \"QR-код\".");
		lv_obj_align(_mew_label1, NULL, LV_ALIGN_IN_TOP_LEFT, 12, 7);

		_mew_btn1 = lv_btn_create(_mew_cont2, NULL);
		lv_obj_set_event_cb(_mew_btn1, _mew_event_handler_1);
		lv_obj_align(_mew_btn1, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 3, -3);

		_mew_btn1_label = lv_label_create(_mew_btn1, NULL);
		lv_obj_set_style(_mew_btn1_label, &_mew_style_txt);
		lv_label_set_text(_mew_btn1_label, "QR-код");
		lv_obj_align(_mew_btn1_label, NULL, LV_ALIGN_CENTER, 0, 0);

		lv_style_copy(&_mew_btn_style_1, &lv_style_btn_rel);
		_mew_btn_style_1.body.main_color = LV_COLOR_MAKE(100, 200, 100);
		_mew_btn_style_1.body.grad_color = LV_COLOR_MAKE(100, 200, 100);
		_mew_btn_style_1.body.border.color = LV_COLOR_MAKE(100, 200, 100);
		_mew_btn_style_1.body.border.width = 1;
		_mew_btn_style_1.body.radius = 2;
		lv_btn_set_style(_mew_btn1, LV_BTN_STYLE_REL, &_mew_btn_style_1);

		_mew_btn2 = lv_btn_create(_mew_cont2, NULL);
		lv_obj_set_event_cb(_mew_btn2, _mew_event_handler_2);
		lv_obj_align(_mew_btn2, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -3, -3);

		_mew_btn2_label = lv_label_create(_mew_btn2, NULL);
		lv_obj_set_style(_mew_btn2_label, &_mew_style_txt);
		lv_label_set_text(_mew_btn2_label, "Позже");
		lv_obj_align(_mew_btn2_label, NULL, LV_ALIGN_CENTER, 0, 0);

		lv_style_copy(&_mew_btn_style_2, &lv_style_btn_rel);
		_mew_btn_style_2.body.main_color = LV_COLOR_MAKE(200, 200, 200);
		_mew_btn_style_2.body.grad_color = LV_COLOR_MAKE(200, 200, 200);
		_mew_btn_style_2.body.border.color = LV_COLOR_MAKE(200, 200, 200);
		_mew_btn_style_2.body.border.width = 1;
		_mew_btn_style_2.body.radius = 2;
		lv_btn_set_style(_mew_btn2, LV_BTN_STYLE_REL, &_mew_btn_style_2);
	}
}


