
#include <LovyanGFX.hpp>
#include "dspText.h"

void dsp_setFont(LovyanGFX *lcd, dspFont font)
{
  lcd->setFont(font.font);
  
  lgfx::v1::TextStyle style;
  style.fore_rgb888 = font.foreColor;
  style.back_rgb888 = font.backColor;
  lcd->setTextStyle(style);

}

dspFont getFont(dspSize _fontSize)
{

  switch (_fontSize)
  {
  case dspSize::size9:
    return font_9;
    break;
  case dspSize::size12:
    return font_12;
    break;
  case dspSize::size18:
    return font_18;
    break;
  case dspSize::size24:
    return font_24;
    break;
  
  default:
    return font_9;
    break;
  };
}

void dsp_setPaletteColor(lgfx::LGFX_Sprite *panel) 
{
  panel->setPaletteColor(dsp_black.index, dsp_black.color);  
  panel->setPaletteColor(dsp_red.index, dsp_red.color);  
  panel->setPaletteColor(dsp_green.index, dsp_green.color);  
  panel->setPaletteColor(dsp_blue.index, dsp_blue.color);  
  panel->setPaletteColor(dsp_yellow.index, dsp_yellow.color);  
  panel->setPaletteColor(dsp_gray.index, dsp_gray.color); 
  panel->setPaletteColor(dsp_white.index, dsp_white.color); 
}

DspText::DspText() {
};

void DspText::init(LovyanGFX *_lcd, int32_t _posx, int32_t _posy, int32_t _width, int32_t _height, dspSize _fontSize)
{
  font = getFont(_fontSize);
  labelFont = getFont(dspSize::size9);
 
  /*
  uint32_t fore_rgb888 = 0xFFFFFFU;
    uint32_t back_rgb888 = 0;
    float size_x = 1;
    float size_y = 1;
    textdatum_t datum = textdatum_t::top_left;
    int32_t padding_x = 0;
    bool utf8 = true;
    bool cp437 = false;
  */
 
  width = _width;
  nchar = (width - (margin * 4)) / font.width;
  posx = _posx;
  posy = _posy;
  height = _height;
  lcd = _lcd;

  frame_x = posx + (margin * 2);
  frame_y = posy + (height / 2) - (font.height / 2) - margin;
  frame_w = width - (margin * 4);
  frame_h = font.height + (margin * 2);

  // dsp_setPaletteColor(&frame);
  
}

void DspText::setBackground(dspColor_t _colorBg, const char *_label, dspSize _fontSize) 
{

  strcpy(label, _label);
  labelFont = getFont(_fontSize);

  setBackground(_colorBg, true);

}

void DspText::setBackground(dspColor_t _colorBg, bool labelChange) 
{

  if (!labelChange && _colorBg.index == colorBg.index) 
    return;

  uint32_t len = strlen(label);
  colorBg = _colorBg;
  font.backColor = colorBg.color;
  uint32_t x = posx;
  uint32_t y = posy;
  

  lcd->startWrite();
  lcd->fillRect(x + margin * 2, y         , width - (margin * 4), margin,           dsp_black.color);
  lcd->fillRect(x + margin, y + margin, width - (margin * 2), height - (margin * 2), colorBg.color);
  lcd->drawRect(x + margin, y + margin, width - (margin * 2), height - (margin * 2), colorMargin.color);

  if (len > 0) {
    len = (len + 1) * labelFont.width;
    lcd->setCursor(x + margin * 2.5, y + margin - labelFont.height);
    dsp_setFont(lcd, labelFont); 
    lcd->print(label);
    dsp_setFont(lcd, font);
  }

  lcd->endWrite();

}


void DspText::clear()
{
  uint32_t x = frame_x;
  uint32_t y = frame_y;

  // frame.setColorDepth(8);  
  // frame.createPalette();
  // frame.createSprite(frame_w, frame_h);
  lcd->fillRect(x, y, frame_w, frame_h, colorBg.color);
  // lcd.pushSprite(lcd , frame_x, frame_y);
  // frame.deleteSprite();
}

void DspText::maskBit(const char * _bit0,const char * _bit1,const char * _bit2,const char * _bit3) 
{
  strcpy(maskBit0, _bit0);
  strcpy(maskBit1, _bit1);
  strcpy(maskBit2, _bit2);
  strcpy(maskBit3, _bit3);

  showBit(false, 0);
  showBit(false, 1);
  showBit(false, 2);
  showBit(false, 3);
}

void DspText::showBit(bool valor, uint8_t statusPanel)
{
  uint32_t offset_x = 52;
  uint32_t offset_y = 18;
  uint32_t bitMargin = 6;

  uint32_t x = posx + (width / 2) - offset_x + bitMargin;
  uint32_t y = posy + (height / 2) - offset_y + bitMargin;
  
  char * label;

  switch (statusPanel)
  {
  case 0:
    x = x - offset_x;
    y = y - offset_y;
    label = maskBit0;
    break;

  case 1:
    x = x + offset_x;
    y = y - offset_y;
    label = maskBit1;
    break;

  case 2:
    x = x - offset_x;
    y = y + offset_y;
    label = maskBit2;
    break;

  case 3:
    x = x + offset_x;
    y = y + offset_y;
    label = maskBit3;
    break;
  
  default:
    break;
  }

  uint32_t color = valor ? dsp_red.color : dsp_gray.color;
  dspFont bitFont = font_9;
  bitFont.backColor = color;


  lcd->fillRect(x, y, (offset_x * 2) - (bitMargin * 2), (offset_y * 2) - (bitMargin * 2), color);
  lcd->setCursor(x + bitMargin, y + bitMargin);
  dsp_setFont(lcd, bitFont);
  lcd->print(label);

}


size_t DspText::write(const uint8_t *buf, size_t size)
{

  uint32_t x = frame_x;
  uint32_t y = frame_y;
  // a esquerda
  // int32_t br = 0;

  // centralizado
   int32_t br = (nchar - size) / 2;
   if (br < 0)
     br = 0;
   if (nchar - (br * 2) > size)
     br++;

  // dsp_setPaletteColor(&frame);
  // frame.setColorDepth(8);  
  // frame.createPalette();
  // frame.createSprite(frame_w, frame_h);
  // frame.setBaseColor(dsp_black.index);
  // frame.clear();

  dsp_setFont(lcd, font);
  
  lcd->setCursor(x, y + (margin / 2));

  for (int c = 0; c < nchar; c++)
  {
    if (c < br || c >= size + br)
      lcd->write(32); // 32
    else
      lcd->write(*buf++);
  }

  // frame.pushSprite(lcd , frame_x, frame_y);
  // frame.pushSprite(lcd , frame_x, frame_y, dsp_black.index);
  // frame.deleteSprite();  
  
  return size;
}

size_t DspText::write(uint8_t utf8)
{
  return lcd->write(utf8);
}

