#include "oled_display.h"

bool oled_display::init(uint16_t _width, uint16_t _height, uint8_t _address, i2c_inst_t *_i2c_instance) {
    width = _width;
    height = _height;
    pages = _height / 8;
    address = _address;

    i2c_i = _i2c_instance;


    bufsize=(pages)*(width);
    if((buffer=(uint8_t*) malloc(bufsize+1))==NULL) {
        bufsize=0;
        return false;
    }

    ++(buffer);

    // from https://github.com/makerportal/rpi-pico-ssd1306
    uint8_t cmds[]= {
        SET_DISP,
        // timing and driving scheme
        SET_DISP_CLK_DIV,
        0x80,
        SET_MUX_RATIO,
        (uint8_t) (height - 1),
        SET_DISP_OFFSET,
        0x00,
        // resolution and layout
        SET_DISP_START_LINE,
        // charge pump
        SET_CHARGE_PUMP,
        (uint8_t) (external_vcc ? 0x10 : 0x14),
        SET_SEG_REMAP | 0x01,           // column addr 127 mapped to SEG0
        SET_COM_OUT_DIR | 0x08,         // scan from COM[N] to COM0
        SET_COM_PIN_CFG,
        (uint8_t) (width > 2 * height ? 0x02 : 0x12),
        // display
        SET_CONTRAST,
        0xff,
        SET_PRECHARGE,
        (uint8_t) (external_vcc ? 0x22 : 0xF1),
        SET_VCOM_DESEL,
        0x30,                           // or 0x40?
        SET_ENTIRE_ON,                  // output follows RAM contents
        SET_NORM_INV,                   // not inverted
        SET_DISP | 0x01,
        // address setting
        SET_MEM_ADDR,
        0x00,  // horizontal
    };

    for(size_t i=0; i<sizeof(cmds); ++i)
        write(cmds[i]);

    return true;
}

void oled_display::deinit() {
    free(buffer-1);
}

void oled_display::poweroff() {
    write(SET_DISP|0x00);
}

void oled_display::poweron() {
    write(SET_DISP|0x01);
}

void oled_display::contrast(uint8_t val) {
    write(SET_CONTRAST);
    write(val);
}

void oled_display::invert(uint8_t inv) {
    write(SET_NORM_INV | (inv & 1));
}

void oled_display::clear() {
    memset(buffer, 0, bufsize);
}

void oled_display::clear_pixel(uint32_t x, uint32_t y) {
    if(x>=width || y>=height) return;

    buffer[x+width*(y>>3)]&=~(0x1<<(y&0x07));
}

void oled_display::draw_pixel(uint32_t x, uint32_t y) {
    if(x>=width || y>=height) return;

    buffer[x+width*(y>>3)]|=0x1<<(y&0x07); // y>>3==y/8 && y&0x7==y%8
}

void oled_display::draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    if(x1>x2) {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }

    if(x1==x2) {
        if(y1>y2)
            swap(&y1, &y2);
        for(int32_t i=y1; i<=y2; ++i)
            oled_display::draw_pixel(x1, i);
        return;
    }

    float m=(float) (y2-y1) / (float) (x2-x1);

    for(int32_t i=x1; i<=x2; ++i) {
        float y=m*(float) (i-x1)+(float) y1;
        oled_display::draw_pixel(i, (uint32_t) y);
    }
}

void oled_display::draw_square(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    for(uint32_t i=0; i<width; ++i)
        for(uint32_t j=0; j<height; ++j)
            oled_display::draw_pixel(x + i, y + j);

}

void oled_display::draw_empty_square(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    oled_display::draw_line(x, y, x + width, y);
    oled_display::draw_line(x, y + height, x + width, y + height);
    oled_display::draw_line(x, y, x, y + height);
    oled_display::draw_line(x + width, y, x + width, y + height);
}

void oled_display::draw_char_with_font(uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c) {
    if(c<font[3]||c>font[4])
        return;

    uint32_t parts_per_line=(font[0]>>3)+((font[0]&7)>0);
    for(uint8_t w=0; w<font[1]; ++w) { // width
        uint32_t pp=(c-font[3])*font[1]*parts_per_line+w*parts_per_line+5;
        for(uint32_t lp=0; lp<parts_per_line; ++lp) {
            uint8_t line=font[pp];

            for(int8_t j=0; j<8; ++j, line>>=1) {
                if(line & 1)
                    oled_display::draw_square(x + w * scale, y + ((lp << 3) + j) * scale, scale, scale);
            }

            ++pp;
        }
    }
}

void oled_display::draw_string_with_font(uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s) {
    for(int32_t x_n=x; *s; x_n+=(font[1]+font[2])*scale) {
        oled_display::draw_char_with_font(x_n, y, scale, font, *(s++));
    }
}

void oled_display::draw_char(uint32_t x, uint32_t y, uint32_t scale, char c) {
    oled_display::draw_char_with_font(x, y, scale, font_8x5, c);
}

void oled_display::draw_string(uint32_t x, uint32_t y, uint32_t scale, const char *s) {
    oled_display::draw_string_with_font(x, y, scale, font_8x5, s);
}

uint32_t oled_display::bmp_get_val(const uint8_t *data, const size_t offset, uint8_t size) {
    switch(size) {
    case 1:
        return data[offset];
    case 2:
        return data[offset]|(data[offset+1]<<8);
    case 4:
        return data[offset]|(data[offset+1]<<8)|(data[offset+2]<<16)|(data[offset+3]<<24);
    default:
        __builtin_unreachable();
    }
    __builtin_unreachable();
}

void oled_display::bmp_show_image_with_offset(const uint8_t *data, const long size, uint32_t x_offset, uint32_t y_offset) {
    if(size<54) // data smaller than header
        return;

    const uint32_t bfOffBits=oled_display::bmp_get_val(data, 10, 4);
    const uint32_t biSize=oled_display::bmp_get_val(data, 14, 4);
    const int32_t biWidth=(int32_t) oled_display::bmp_get_val(data, 18, 4);
    const int32_t biHeight=(int32_t) oled_display::bmp_get_val(data, 22, 4);
    const uint16_t biBitCount=(uint16_t) oled_display::bmp_get_val(data, 28, 2);
    const uint32_t biCompression=oled_display::bmp_get_val(data, 30, 4);

    if(biBitCount!=1) // image not monochrome
        return;

    if(biCompression!=0) // image compressed
        return;

    const int table_start=14+biSize;
    uint8_t color_val;

    for(uint8_t i=0; i<2; ++i) {
        if(!((data[table_start+i*4]<<16)|(data[table_start+i*4+1]<<8)|data[table_start+i*4+2])) {
            color_val=i;
            break;
        }
    }

    uint32_t bytes_per_line=(biWidth/8)+(biWidth&7?1:0);
    if(bytes_per_line&3)
        bytes_per_line=(bytes_per_line^(bytes_per_line&3))+4;

    const uint8_t *img_data=data+bfOffBits;

    int step=biHeight>0?-1:1;
    int border=biHeight>0?-1:biHeight;
    for(uint32_t y=biHeight>0?biHeight-1:0; y!=border; y+=step) {
        for(uint32_t x=0; x<biWidth; ++x) {
            if(((img_data[x>>3]>>(7-(x&7)))&1)==color_val)
                oled_display::draw_pixel(x_offset + x, y_offset + y);
        }
        img_data+=bytes_per_line;
    }
}

void oled_display::bmp_show_image(const uint8_t *data, const long size) {
    bmp_show_image_with_offset(data, size, 0, 0);
}

void oled_display::show() {
    uint8_t payload[]= {SET_COL_ADDR, 0, (uint8_t) (width - 1), SET_PAGE_ADDR, 0, (uint8_t) (pages - 1)};
    if(width==64) {
        payload[1]+=32;
        payload[2]+=32;
    }

    for(size_t i=0; i < sizeof(payload); ++i)
        write(payload[i]);

    *(buffer-1)=0x40;

    fancy_write(i2c_i, address, buffer-1, bufsize+1, "oled_display::show");
}
