
#include <LovyanGFX.hpp>
#include "button.h"

Button::Button() {
};

void Button::init(LovyanGFX *_lcd, int _posx, int _posy, const char *_label, bool _hidden)
{
  lcd = _lcd;
  posx = _posx;
  posy = _posy;
  height = 100; // // 220 100
  width = 220;
  hidden = _hidden;

  setLabel(_label);
 
}

void Button::setLabel(const char *_label)
{
  if (hidden == false)
  {
    int32_t lbX = posx + (width / 2) - (strlen(_label) * 26 / 2); // centralizando texto
    lcd->setCursor(lbX, posy + 16);
    lcd->print(_label);
    lcd->drawRect(posx, posy, width, height, 0x555F);
  }
}

bool Button::pressed(int32_t cursorX, int32_t cursorY)
{
  if (cursorX >= posx && cursorX <= posx + width && cursorY >= posy && cursorY <= posy + height)
    return true;
  else
    return false;
}