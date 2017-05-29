#ifndef _ILI9341_
#define _ILI9341_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <libopencm3/stm32/spi.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>

#include "utf8_to_cp1251.h"
#include "board.h"

#define STATUSBAR_BUF               1
#define MENU_ELEMENT_BUF            2

#define SCREEN_W                    240
#define SCREEN_H                    320
#define STATUSBAR_W                 20
#define MENU_ELEMENTS_COUNT         10
#define STATUSBAR_BUF_SIZE          ((SCREEN_H * STATUSBAR_W) * 2)      // 12800
#define ELEMENT_H                   (SCREEN_H / MENU_ELEMENTS_COUNT)
#define ELEMENT_W                   (SCREEN_W - STATUSBAR_W)
#define ELEMENT_BUF_SIZE            ((ELEMENT_H * ELEMENT_W) * 2)       // 14080

#define FONT_0_DEG                  1
#define FONT_90_DEG                 2
#define FONT_180_DEG                3
#define FONT_270_DEG                4

#define FONT_W                      5
#define FONT_H                      7
#define FONT_SPACE                  1
#define LINE_SPACE                  4

#define MAX_CHARS_ON_LINE           (SCREEN_W / (FONT_W + FONT_SPACE))
#define MAX_CHARS_LINES             5

#define BG_COLOR                    0x00

#define SMALL_WAIT                  10
#define PIN_SET_CLEAR_WAIT          10000
#define RESET_WAIT                  70000

#define DC_PIN                      GPIO0
#define DC_PORT                     GPIOC
#define RESET_PIN                   GPIO2
#define RESET_PORT                  GPIOC
#define CS_PIN                      GPIO1
#define CS_PORT                     GPIOC

#define __display_cs_clear          gpio_clear(CS_PORT, CS_PIN)
#define __display_cs_set            gpio_set(CS_PORT, CS_PIN)
#define __display_dc_clear          gpio_clear(DC_PORT, DC_PIN)
#define __display_dc_set            gpio_set(DC_PORT, DC_PIN)
#define __display_reset_set         gpio_clear(RESET_PORT, RESET_PIN)
#define __display_reset_clear       gpio_set(RESET_PORT, RESET_PIN)

extern void display_setup(void);
extern void display_fill(u16 r, u16 g, u16 b);
extern void display_rect_fill(u16 x, u16 y, u16 w, u16 h, u16 r, u16 g, u16 b);
extern void display_write_page(u16 x, u16 y, u16 w, u16 h, u8* buf, u16 size);

extern void direct_draw_char(u8 ascii, u16 x, u16 y, u16 size, u16 r, u16 g, u16 b);
extern void direct_draw_string(u8* ascii, u16 x, u16 y, u16 size, u16 r, u16 g, u16 b);
extern void direct_draw_string_ml(u8* ascii, u16 x, u16 y, u16 frame_w, u16 frame_h, u16 size, u16 r, u16 g, u16 b);

extern void g_commit(u8 buf_id, u8 item_id);
extern void g_set_pixel(u8 buf_id, u16 x, u16 y, u16 r, u16 g, u16 b);
extern void g_draw_vline(u8 buf_id, u16 x1, u16 y1, u16 y2, u16 r, u16 g, u16 b);
extern void g_draw_hline(u8 buf_id, u16 x1, u16 x2, u16 y1, u16 r, u16 g, u16 b);
extern void g_clear_buf(u8 buf_id, u16 r, u16 g, u16 b);
extern void g_draw_string(u8 buf_id, u8* ascii, u16 x, u16 y, u16 size, u16 r, u16 g, u16 b, u8 dir);

#endif
