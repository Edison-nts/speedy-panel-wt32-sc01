#include <LovyanGFX.hpp>

#ifndef Button_h
#define Button_h


class Button
{
public:
  Button();
  void init(LovyanGFX *_lcd, int32_t _posx, int32_t _posy, const char *_label, bool hidden);
  void setLabel(const char *_label);
  bool pressed(int32_t cursorX, int32_t cursorY);
  // void print(char* data);

private:
  int32_t posx;
  int32_t width;
  int32_t posy;
  int32_t height;
  bool hidden;
  LovyanGFX *lcd;
};

#endif