
#include "LGFX.hpp"

static LGFX lcd;

#include "button.h"
#include "dspText.h"

#define SerialEcu Serial0 // <- Serial Debug
#define SerialDash Serial // <- Serial USB

static unsigned long lastTime = 0;
static uint8_t counter = 0;
static DspText dspTeste;
static Button btnEnviar;
bool running = true;

// posicao atual do cursor
static int32_t x, y;
static int32_t cursorX;
static int32_t cursorY;
static bool touching;

void readButton();

void setup(void)
{
  // Serial.begin(115200); // <- USB
  SerialEcu.begin(115200); // <- debug pins rx e tx
  lcd.init();

  lcd.setRotation(3);

  lcd.setFont(&fonts::FreeMono9pt7b);

  // init botoes
  btnEnviar.init(&lcd, 170, 10, "STOP ", false);
  // ---------------------------"START"

  // init caixas texto
  dspTeste.init(&lcd, 0, 0, "contador");
}

void loop()
{
  unsigned long currentTime = micros();
  bool read = true;

  if (SerialEcu.available())
  {
    read = false;
    char c = SerialEcu.read();
    SerialDash.write(c);
  }

  if (SerialDash.available())
  {
    read = false;
    char c = SerialDash.read();
    SerialEcu.write(c);
  }

  if (read == true)
  {
    readButton();
  }
}

void readButton()
{

  if (lcd.getTouch(&x, &y))
  {
    if (abs(cursorX - x) > 10 || abs(cursorY - y) > 10)
    {
      cursorX = x;
      cursorY = y;
      touching = true;
    }
  }
  else
  {
    if (touching == true)
    {
      touching = false;
      if (btnEnviar.pressed(cursorX, cursorY))
      {
        if (running)
        {
          running = false;
          SerialEcu.println(" ... Start pressed, teste 2");
          btnEnviar.setLabel("START");
        }
        else
        {
          running = true;
          SerialEcu.println(" ... Stop pressed, teste 2");
          btnEnviar.setLabel("STOP ");
        }
      }
      cursorX = 0;
      cursorY = 0;
    }
  }
}