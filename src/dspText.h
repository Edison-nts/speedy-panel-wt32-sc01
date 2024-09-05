#include <LovyanGFX.hpp>
#include <Print.h>

#ifndef DspText_h
#define DspText_h
#define DSP_MARGIN 10
#define DSP_OFFSETY 80

 enum dspSize {
    size9 = 9,
    size12 = 12,
    size18 = 18,
    size24 = 24
 };

 struct dspColor_t {
  size_t index;
  uint32_t color;
 };

struct dspFont {
  const lgfx::GFXfont * font;
  uint8_t width;
  uint8_t height;
  uint32_t foreColor;
  uint32_t backColor;
};

const dspColor_t dsp_black = { 0 , 0x000000U};
const dspColor_t dsp_red = {1, 0x770000U};
const dspColor_t dsp_green = {2, 0x007700U}; 
const dspColor_t dsp_blue = {3, 0x000077U};
const dspColor_t dsp_yellow = {4, 0x999900U};
const dspColor_t dsp_cyan = {5, 0x009999U};
const dspColor_t dsp_roxo = {6, 0x990099U};
const dspColor_t dsp_gray = {7, 0x555555U};

const dspColor_t dsp_lite_green = {8, 0x55FF55U};
const dspColor_t dsp_lite_red = {9, 0xFF5555U};
const dspColor_t dsp_lite_blue = {10, 0x5555FFU};
const dspColor_t dsp_lite_gray = {11, 0xAAAAAAU};

const dspColor_t dsp_white = {16, 0xFFFFFFU};


const dspFont font_9 = {
  &lgfx::v1::fonts::FreeMono9pt7b,
  11,
  9,
  0xFFFFFFU,
  0x000000U
};

const dspFont font_12 = {
  &lgfx::v1::fonts::FreeMono12pt7b,
  14,
  11,
  0xFFFFFFU,
  0x000000U
};

const dspFont font_18 = {
  &lgfx::v1::fonts::FreeMono18pt7b,
  20,
  17,
  0xFFFFFFU,
  0x000000U
};

const dspFont font_24 = {
  &lgfx::v1::fonts::FreeMono24pt7b,
  28,
  32,
  0xFFFFFFU,
  0x000000U
};

dspFont getFont(dspSize _fontSize);

void dsp_setPaletteColor(lgfx::LGFX_Sprite * panel);

void dsp_setFont(LovyanGFX *_lcd, dspFont _font);

class DspText : public Print
{
public:
  DspText();
  void init(LovyanGFX *_lcd, int32_t _posx, int32_t _posy, int32_t _width, int32_t _height, dspSize _fontSize);
  size_t write(const uint8_t *buf, size_t size);
  size_t write(uint8_t utf8);
  void showBit(bool valor, uint8_t statusPanel);
  void maskBit(const char * _bit0,const char * _bit1,const char * _bit2,const char * _bit3);
  void setBackground(dspColor_t _colorBg, const char *_label, dspSize _fontSize);
  void setBackground(dspColor_t _colorBg, bool labelChange);
  void clear();
private:
  int32_t posx;
  int32_t width;
  const int32_t margin = DSP_MARGIN;
  char label[32];
  dspFont labelFont;

  dspFont font;
  int32_t posy;
  int32_t height;
  int32_t nchar;
  dspColor_t colorMargin = dsp_white;
  dspColor_t colorBg = dsp_blue;
  LovyanGFX * lcd;
  // lgfx::LGFX_Sprite frame;

  uint32_t frame_x;
  uint32_t frame_y;
  uint32_t frame_w;
  uint32_t frame_h;

  char maskBit0[32];
  char maskBit1[32];
  char maskBit2[32];
  char maskBit3[32];
  
};

#endif