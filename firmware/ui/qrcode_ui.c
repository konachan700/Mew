#include "ui.h"
#include "debug.h"
#include "drivers/system/system.h"
#include "drivers/flash/flash.h"
#include "app/qr_code/qrcodegen.h"
#include "drivers/display/display.h"

#include "lv_conf.h"
#include "lvgl/lvgl.h"

#define DOTSIZE 6
#define X_OFFSET 21
#define Y_OFFSET 101

static uint8_t _mew_qr_buffer[qrcodegen_BUFFER_LEN_FOR_VERSION(4)];
static uint8_t _mew_qr_data[qrcodegen_BUFFER_LEN_FOR_VERSION(4)];

static lv_color_t _mew_gd[MEW_DISPLAY_W * DOTSIZE];

void __mew_setpoint(uint8_t x, uint8_t y) {
	uint32_t i, xp, line;
	for (i=0; i<DOTSIZE; i++) {
		xp   = (x * DOTSIZE) + X_OFFSET;
		line = (i * MEW_DISPLAY_W);
		memset((void*)(_mew_gd + (line + xp)), 0, DOTSIZE * sizeof(lv_color_t));
	}
}

void mew_qrcode_set_data(uint8_t* data, uint32_t offset, uint32_t len) {
	memcpy((void*)(_mew_qr_data + offset), data, len);
}

void mew_display_qrcode(void) {
	uint8_t x, y;
	uint32_t ypos, size;

	qrcodegen_encodeBinary(_mew_qr_data, 64, _mew_qr_buffer, qrcodegen_Ecc_LOW, 4, 4, qrcodegen_Mask_AUTO, true);
	size = qrcodegen_getSize(_mew_qr_buffer);

	for (y=0; y<size; y++) {
		memset((void*) _mew_gd, 0xFF, (MEW_DISPLAY_W * DOTSIZE) * sizeof(lv_color_t));
		for (x=0; x<size; x++) {
			if (qrcodegen_getModule(_mew_qr_buffer, x, y)) {
				__mew_setpoint(x, y);
			}
		}
		ypos = (y * DOTSIZE) + Y_OFFSET;
		mew_display_flush_sync_ext(
				0,
				ypos,
				MEW_DISPLAY_W - 1,
				ypos + (DOTSIZE - 1),
				_mew_gd);
	}
}
