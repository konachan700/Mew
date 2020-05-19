#ifndef __MEW_UI_LHANDLERS__
#define __MEW_UI_LHANDLERS__

#include "mew.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lv_conf.h"
#include "lvgl/lvgl.h"

#define MEW_PINCODE_MAX_SIZE 8
#define MEW_PINCODE_MAGIC_ADD 0x43

#define MEW_WINDOW_ZERO 			0x00
#define MEW_WINDOW_INITIAL_QR 		0x01
#define MEW_WINDOW_MAIN_WIN			0x02

#define MEW_WINDOW_STARTUP_1		0xA1


unsigned int mew_lvgl_lhandler(void);
unsigned int mew_ui_lhandler(void);

void mew_ui_show_window(uint32_t win);
void mew_ui_header_set_text(char* text);
void mew_remove_ui_header(void);
void mew_display_small_header(void);
void mew_show_startup_window_1(void);
void mew_display_ui_header(char* text);
void mew_qrcode_set_data(uint8_t* data, uint32_t offset, uint32_t len);
void mew_display_qrcode(void);
void mew_ui_msgbox(const char * text, lv_event_cb_t evt);
void mew_show_main_window(void);

#endif
