#include "ILI9341.h"

u8 item_framebuffer[ELEMENT_BUF_SIZE];
u8 stbar_framebuffer[STATUSBAR_BUF_SIZE];
u8* char_buf;
u16 char_buf_size;

void __wait(u32 delay) { 
    u32 i;
    for (i=0; i<delay; i++) __asm__("NOP");
}

void __data_set(void) {
    __display_dc_set;
    __display_cs_clear;
    __wait(PIN_SET_CLEAR_WAIT);
}

void __cmd_set(void) {
    __display_dc_clear;
    __display_cs_clear;
    __wait(PIN_SET_CLEAR_WAIT);
}

void __display_reset(void) {
    __display_cs_set;
    __display_dc_set;
    __display_reset_set;
    __wait(RESET_WAIT);
    __display_reset_clear;
}

void __spi_write_cmd(u8 index) {
	__cmd_set();
    spi_xfer_dma(&index, 0x01, 1);
    //__display_cs_set;
}

void __spi_write_cmd_w_data(u8 index, u8* data, u16 count) {
    __cmd_set();
    spi_xfer_dma(&index, 1, 1);
    __data_set();
    spi_xfer_dma(data, count, 1);
    //__display_cs_set;
}

void __set_regoin(u16 x, u16 y, u16 w, u16 h) {
    __spi_write_cmd_w_data(0x2A, (unsigned char[]) { (x >> 8), x, ((x+w-1) >> 8), (x+w-1) }, 4);
    __spi_write_cmd_w_data(0x2B, (unsigned char[]) { (y >> 8), y, ((y+h-1) >> 8), (y+h-1) }, 4);
}

void display_write_page(u16 x, u16 y, u16 w, u16 h, u8* buf, u16 size) {
    __set_regoin(x, y, w, h);
    __spi_write_cmd(0x2c);
    __data_set();
    spi_xfer_dma(buf, size, 1);
    //__display_cs_set;
}

void display_rect_fill(u16 x, u16 y, u16 w, u16 h, u16 r, u16 g, u16 b) {
    u16 i;
    signed int k;
    u16 color565 = ((u16) (((r >> 3 ) << 11) | ((g >> 2 ) << 5) | (b  >> 3)));

    __set_regoin(x, y, w, h);
    __spi_write_cmd(0x2c);
    __data_set();
		
    k = (h * w * 2);
    if (k > 65535) {
        while (1) {
            spi_xfer_dma(NULL, ((k > 65535) ? 65535 : k), 1);
            k -= 65535;
            if (k < 0) return;
        }
    } else 
        spi_xfer_dma(NULL, k, 1);
}

void display_fill(u16 r, u16 g, u16 b) {
    display_rect_fill(0, 0, 240, 320, r, g, b);
}

void g_commit(u8 buf_id, u8 item_id) {
    switch (buf_id) {
        case STATUSBAR_BUF:
            display_write_page(0, 0, STATUSBAR_W, SCREEN_H, stbar_framebuffer, STATUSBAR_BUF_SIZE);
            break;
        case MENU_ELEMENT_BUF:
            display_write_page(STATUSBAR_W, (ELEMENT_H * item_id), ELEMENT_W, ELEMENT_H, item_framebuffer, ELEMENT_BUF_SIZE);
            break;
    }
}

void g_clear_buf(u8 buf_id, u16 r, u16 g, u16 b) {
    u16 i;
    u16 color565 = ((u16) (((r >> 3 ) << 11) | ((g >> 2 ) << 5) | (b  >> 3)));
    
    switch (buf_id) {
        case STATUSBAR_BUF:
            for (i=0; i<STATUSBAR_BUF_SIZE; i=i+2) { 
                stbar_framebuffer[i]   = (color565 >> 8) & 0xFF;
                stbar_framebuffer[i+1] = color565 & 0xFF;
            }
            break;
        case MENU_ELEMENT_BUF:
            for (i=0; i<ELEMENT_BUF_SIZE; i=i+2) { 
                item_framebuffer[i]   = (color565 >> 8) & 0xFF;
                item_framebuffer[i+1] = color565 & 0xFF;
            }
            break;
    }
}

// INFO: its prevent buffer overflow
void __buf_set_8(u8* buf, u16 buf_size, u8 data, u16 index) {
    if (index >= buf_size) return;
    buf[index] = data;
}

void g_set_pixel(u8 buf_id, u16 x, u16 y, u16 r, u16 g, u16 b) {
    u16 color565 = ((u16) (((r >> 3 ) << 11) | ((g >> 2 ) << 5) | (b  >> 3)));
    
    u16 buf_w, buf_h, buf_sz;
    u8* buf;
    
    switch (buf_id) {
        case STATUSBAR_BUF:
            buf_w = STATUSBAR_W;
            buf_h = SCREEN_H;
            buf = stbar_framebuffer;
            buf_sz = STATUSBAR_BUF_SIZE;
            break;
        case MENU_ELEMENT_BUF:
            buf_w = ELEMENT_W;
            buf_h = ELEMENT_H;
            buf = item_framebuffer;
            buf_sz = ELEMENT_BUF_SIZE;
            break;
    }

    __buf_set_8(buf, buf_sz, (u8) ((color565 >> 8) & 0xFF), (buf_w * 2 * y) + (x * 2));
    __buf_set_8(buf, buf_sz, (u8) (color565 & 0xFF), (buf_w * 2 * y) + (x * 2) + 1);
}

void g_draw_vline(u8 buf_id, u16 x1, u16 y1, u16 y2, u16 r, u16 g, u16 b) {
    u16 i;
    for (i=y1; i<y2; i++) g_set_pixel(buf_id, x1, i, r, g, b);
}

void g_draw_hline(u8 buf_id, u16 x1, u16 x2, u16 y1, u16 r, u16 g, u16 b) {
    u16 i;
    for (i=x1; i<x2; i++) g_set_pixel(buf_id, i, y1, r, g, b);
}


void __direct_set_pixel_for_char(u8* buf, u16 buf_sz, u16 x, u16 y, u16 r, u16 g, u16 b, u16 buf_w) {
    u16 color565 = ((u16) (((r >> 3 ) << 11) | ((g >> 2 ) << 5) | (b  >> 3)));
    __buf_set_8(buf, buf_sz, (u8) ((color565 >> 8) & 0xFF), (buf_w * 2 * y) + (x * 2));
    __buf_set_8(buf, buf_sz, (u8) (color565 & 0xFF), (buf_w * 2 * y) + (x * 2) + 1);
}

// WARNING: its too slowly
void __direct_draw_char(u8 ascii, u16 x, u16 y, u16 size, u16 r, u16 g, u16 b, u16 buf_size) {
    u16 color565 = ((u16) (((r >> 3 ) << 11) | ((g >> 2 ) << 5) | (b  >> 3)));
    u8 c;
    u16 a, n, z, e;

    if (char_buf == NULL) return;
    for (a=0; a<buf_size; a++) char_buf[a] = 0x00;
    
    for (a=0; a<FONT_W; a++) {
        c = font_ru[(ascii * FONT_W) + a];
        for (n=0; n<FONT_H; n++) {
            if ((c >> n) & 0x01) {
                if (size <= 1) {
                    __direct_set_pixel_for_char(char_buf, buf_size, a, n, r, g, b, size * FONT_W);
                } else {
                    for (z=0; z<size; z++) { 
                        for (e=0; e<size; e++) {
                            __direct_set_pixel_for_char(char_buf, buf_size, (a * size) + z, (n * size) + e, r, g, b, size * FONT_W);
                        }
                    }
                }
            }
        }
    }

    //display_write_page(x, y, (FONT_W * size), (FONT_H * size), char_buf, buf_size);
    display_write_page(x, y, (FONT_W * size), (FONT_H * size), char_buf, buf_size);
    //free(char_buf);
}

void direct_draw_string(u8* ascii, u16 x, u16 y, u16 size, u16 r, u16 g, u16 b) {
    u16 i;
    int len;
    u8 text_buf[MAX_CHARS_ON_LINE];
    
    len = convertUtf8ToCp1251(ascii, text_buf);
    if (len == -1) return;
    
    char_buf_size = (size * FONT_W) * (FONT_H * size) * 2;
    char_buf = (u8*) malloc(char_buf_size);
    
    for (i=0; i<len; i++) {
        __direct_draw_char(text_buf[i], x + (FONT_SPACE * size * i) + (FONT_W * size * i), y, size, r, g, b, char_buf_size);
    }
    
    free(char_buf);
}

void direct_draw_string_ml(u8* ascii, u16 x, u16 y, u16 frame_w, u16 frame_h, u16 size, u16 r, u16 g, u16 b) {
    u16 i, x_pos, curr_line = 0, curr_char = 0;
    int len;
    u8 text_buf[MAX_CHARS_ON_LINE];
    
    len = convertUtf8ToCp1251(ascii, text_buf);
    if (len == -1) return;
    
    char_buf_size = (size * FONT_W) * (FONT_H * size) * 2;
    char_buf = (u8*) malloc(char_buf_size);
    
    for (i=0; i<len; i++) {
        x_pos = ((FONT_SPACE * size * curr_char) + (FONT_W * size * curr_char));
        if ((text_buf[i] == '\n') || (x_pos >= frame_w)) {
            curr_line++;
            curr_char = 0;
            x_pos = ((FONT_SPACE * size * curr_char) + (FONT_W * size * curr_char));
        }

        if (text_buf[i] != '\n') {
            __direct_draw_char(text_buf[i], x + x_pos, y + (LINE_SPACE * size * curr_line) + (FONT_H * size * curr_line), size, r, g, b, char_buf_size);
            curr_char++;
        }
    }
    
    free(char_buf);
}

#ifdef __FONT_STM32_MEW_ICONS__
void g_draw_icon(u8 buf_id, const u8* icon, u16 xn, u16 yn, u16 r, u16 g, u16 b) {
    u8 x, y, bit = 0, icon_byte = 0;
    for (y=0; y<MEW_ICONS_FONT_HEIGHT; y++) {
        for (x=0; x<MEW_ICONS_FONT_WIDTH; x++) {
            if (bit > 7) {
                bit = 0;
                icon_byte++;
            }
            
            if ((icon[icon_byte] & (1 << bit)) != 0)
                g_set_pixel(buf_id, x + xn, y + yn, r, g, b);
            
            bit++;
        }
    }
}
#endif

// WARNING: modes FONT_180_DEG and FONT_270_DEG is not tested!
void g_draw_char(u8 buf_id, u8 ascii, u16 x, u16 y, u16 size, u16 r, u16 g, u16 b, u8 dir) {
    u8 a, n, c, i, z, e;
    
    for (a=0; a<FONT_W; a++) {
        c = font_ru[(ascii * FONT_W) + a];
        for (n=0; n<FONT_H; n++) {
            if ((c >> n) & 0x01) {
                if (size <= 1) {
                    switch (dir) {
                        case FONT_0_DEG:
                            g_set_pixel(buf_id, x + a, y + n, r, g, b);
                            break;
                        case FONT_90_DEG:
                            g_set_pixel(buf_id, x + (FONT_H - n - 1), y + a, r, g, b);
                            break;
                        case FONT_180_DEG:
                            g_set_pixel(buf_id, x + FONT_W - a, y + FONT_H - n, r, g, b);
                            break;
                        case FONT_270_DEG:
                            g_set_pixel(buf_id, x + FONT_H - (FONT_H - n - 1), y + FONT_W - a, r, g, b);
                            break;
                    }
                } else {
                    for (z=0; z<size; z++) { 
                        for (e=0; e<size; e++) {
                            switch (dir) {
                                case FONT_0_DEG:
                                    g_set_pixel(buf_id, x + (a * size) + z, y + (n * size) + e, r, g, b);
                                    break;
                                case FONT_90_DEG:
                                    g_set_pixel(buf_id, x + ((FONT_H - n - 1) * size) + z, y + (a * size) + e, r, g, b);
                                    break;
                                case FONT_180_DEG:
                                    g_set_pixel(buf_id, x + (FONT_W * size) - (a * size) - z - 1, y + (FONT_H * size) - (n * size) - e - 1, r, g, b);
                                    break;
                                case FONT_270_DEG:
                                    g_set_pixel(buf_id, x + (FONT_H * size) - ((FONT_H - n - 1) * size) - z - 1, y + (FONT_W * size) - (a * size) - e - 1, r, g, b);
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void g_draw_string(u8 buf_id, u8* ascii, u16 x, u16 y, u16 size, u16 r, u16 g, u16 b, u8 dir) {
    u16 i;
    int len;
    u8 text_buf[MAX_CHARS_ON_LINE];
    
    len = convertUtf8ToCp1251(ascii, text_buf);
    if (len == -1) return;
    
    for (i=0; i<len; i++) {
        switch (dir) {
            case FONT_0_DEG:
                g_draw_char(buf_id, text_buf[i], x + (FONT_SPACE * size * i) + (FONT_W * size * i), y, size, r, g, b, dir);
                break;
            case FONT_90_DEG:
                g_draw_char(buf_id, text_buf[i], x, y + (FONT_SPACE * size *  i) + (FONT_W * size * i), size, r, g, b, dir);
                break;
            case FONT_180_DEG:
                g_draw_char(buf_id, text_buf[i], x + (FONT_SPACE * (len - i - 1)) + (FONT_W * (len - i - 1)), y, size, r, g, b, dir);
                break;
            case FONT_270_DEG:
                g_draw_char(buf_id, text_buf[i], x, y + (FONT_SPACE * size * (len - i - 1)) + (FONT_W * size * (len - i - 1)), size, r, g, b, dir);
                break;
        }
    }
}

void display_setup(void) {
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_MODE_OUTPUT, GPIO0 | GPIO1 | GPIO2);
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO0 | GPIO1 | GPIO2);
    
    __display_reset();
    
    __spi_write_cmd_w_data(0xCB, (char[]) { 0x39,0x2C,0x00,0x34,0x02 }, 5);
    __spi_write_cmd_w_data(0xCF, (char[]) { 0x00,0xC1,0x30 }, 3);
    __spi_write_cmd_w_data(0xE8, (char[]) { 0x85,0x00,0x78 }, 3);
    __spi_write_cmd_w_data(0xEA, (char[]) { 0x00,0x00 }, 2);
    __spi_write_cmd_w_data(0xED, (char[]) { 0x64,0x03,0x12,0x81 }, 4);
    __spi_write_cmd_w_data(0xF7, (char[]) { 0x20 }, 1);
    __spi_write_cmd_w_data(0xC0, (char[]) { 0x23 }, 1);
    __spi_write_cmd_w_data(0xC1, (char[]) { 0x10 }, 1);
    __spi_write_cmd_w_data(0xC5, (char[]) { 0x3e,0x28 }, 2);
    __spi_write_cmd_w_data(0xC7, (char[]) { 0x86 }, 1);
    __spi_write_cmd_w_data(0x36, (char[]) { 0x48 }, 1);
    __spi_write_cmd_w_data(0x3A, (char[]) { 0x55 }, 1);
    __spi_write_cmd_w_data(0xB1, (char[]) { 0x00,0x18 }, 2);
    __spi_write_cmd_w_data(0xB6, (char[]) { 0x08,0x82,0x27 }, 3);
    __spi_write_cmd_w_data(0xF2, (char[]) { 0x00 }, 1);
    __spi_write_cmd_w_data(0x26, (char[]) { 0x01 }, 1);
    __spi_write_cmd_w_data(0xE0, (char[]) { 0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00 }, 15);
    __spi_write_cmd_w_data(0xE1, (char[]) { 0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F }, 15);
    __spi_write_cmd_w_data(0x33, (char[]) { 0x00,0x00,0x01,0x40,0x00,0x00 }, 6);
    __spi_write_cmd_w_data(0x37, (char[]) { 0x00,0x00 }, 2);

    __spi_write_cmd(0x11);
    __wait(PIN_SET_CLEAR_WAIT);
    __spi_write_cmd(0x29);
    
    //spi_disable(SPI2);
    //spi_set_baudrate_prescaler(SPI2, SPI_CR1_BR_FPCLK_DIV_2);
    //spi_enable(SPI2);
}
