#include "global.h"
#include "LGFX.hpp"
#include "button.h"
#include "dspText.h"

// https://lovyangfx.readthedocs.io/en/latest/02_using.html
static LGFX lcd;
static uint8_t printLoop = 0;

static int8_t telaIndex = 0;
static DspText dsp_00;
static DspText dsp_01;
static DspText dsp_10;
static DspText dsp_11;
 
static Button btnEnviar;
static Button btnStartLog;

// posicao atual do cursor
static int32_t x, y;
static int32_t cursorX;
static int32_t cursorY;
static bool touching;

static uint8_t lastSdStatus = SD_STATUS_OFF;

void displaySetup(statuses* status) {
  lcd.init();

  lcd.setColorDepth(8);

  lcd.setRotation(3);

  dsp_setFont(&lcd, font_24);
   
  printTitulo("Speed Monitor 1");

  // init botoes
  btnEnviar.init(&lcd, 320, 10, ">>>", true);
  // ---------------------------"START "

  /*
  DspText init, posx: 8, posy: 96, width: 230, height: 90 
  DspText init, posx: 8, posy: 216, width: 230, height: 90 
  DspText init, posx: 238, posy: 96, width: 230, height: 90 
  DspText init, posx: 238, posy: 216, width: 230, height: 90 
  */
  btnStartLog.init(&lcd, 10, 100, "Start", true); 
  dsp_00.init(&lcd,   0,  90, 240, 120, dspSize::size24);
  dsp_01.init(&lcd,   0, 210, 240, 120, dspSize::size24);
  dsp_10.init(&lcd, 230,  90, 240, 120, dspSize::size24);
  dsp_11.init(&lcd, 230, 210, 240, 120, dspSize::size24);

  dsp_00.setBackground(dsp_blue, "RPM", dspSize::size9); // RPM
  dsp_01.setBackground(dsp_blue, "TPS", dspSize::size9);        // TPS
  dsp_10.setBackground(dsp_blue, "MAP", dspSize::size9);
  dsp_11.setBackground(dsp_black, "CLT", dspSize::size9);
  lastSdStatus = status->sdCardStatus;

  showCommMonitor(status);

}


void showCommMonitor(statuses* status) 
{
  uint32_t color = dsp_gray.color;
  
  if (status->receiveCount % 2 == 0 ) {
    color = status->receiveCount % 4 == 0 ? dsp_gray.color : dsp_lite_red.color;
    lcd.fillRect(440, 20, 10, 10, color);
  }

  if (status->transmitCount % 2 == 0) {
    color = status->transmitCount % 4 == 0 ?  dsp_gray.color : dsp_lite_green.color;
    lcd.fillRect(440, 40, 10, 10, color); 
  }
  
}

void printTitulo(const char *linha) {
  dsp_setFont(&lcd, font_18);
  lcd.setCursor(30, 30);
  lcd.print(linha);
}

void showData(statuses* status)
{
  status->currentStatusChanged = false;
  printLoop++;

  // lcd->startWrite();
  // dsp_00.clear();
  // dsp_01.clear();
  // dsp_10.clear();
  // dsp_11.clear(); 
  // lcd->endWrite();
   
  // dspInfo.printf(" ... result %d bytes", len);
  // dspLoop.printf("%d", status.secl);

  lcd.startWrite();
  if (telaIndex == 0) {
    dsp_00.printf("%4d", status->RPM);
    dsp_01.printf("%4.1f", status->TPS / 2.0);
    dsp_10.printf("%4d", status->MAP);
    dsp_11.printf("%4d", status->coolant);
    dsp_11.setBackground(BIT_CHECK(status->status4, 3) ? dsp_red : dsp_green, false); // fan
  }
  else if (telaIndex == 1) {
    dsp_00.printf("%4d", status->IAT);
    dsp_01.printf("%4.2f", status->O2 / (10.0 * 14.7));
    dsp_10.printf("%4d", status->actualDwell);
    dsp_11.printf("%4d", status->advance);
  }
  else if (telaIndex == 2) {
    dsp_00.printf("%4.1f", status->battery10 / 10.0);
    dsp_01.printf("%4d", status->PW1);
    dsp_10.printf("%4.1f", status->oilPressure * 0.06894757);
    dsp_11.printf("%4.1f", status->fuelPressure * 0.06894757); 
  }
  else if (telaIndex == 3) {
    dsp_00.printf("%4d", status->corrections);
    dsp_01.printf("%4d", status->AEamount);
    dsp_10.printf("%4d", status->wueCorrection);
    dsp_11.printf("%4d", status->egoCorrection); 
  }
  else if (telaIndex == 4) {

    if (lastSdStatus != status->sdCardStatus) {
      dsp_00.setBackground(status->sdCardStatus == SD_STATUS_ACTIVE ? dsp_red : dsp_gray, false);
    }
    dsp_00.printf("%s", status->sdCardStatus == SD_STATUS_ACTIVE ? "Stop": "Start");
    
    dsp_01.showBit(BIT_CHECK(status->engine, 0), 0); // running
    dsp_01.showBit(BIT_CHECK(status->engine, 1), 1); // crank
    dsp_01.showBit(BIT_CHECK(status->engine, 3), 2); // warmup
    dsp_01.showBit(BIT_CHECK(status->spark, 7), 3); // sync

    bool flipCounting = false;
    if (status->sdCardStatus == SD_STATUS_ACTIVE && status->sdCardblockCount % 8 < 4) {
      flipCounting = true;
    }
     
    dsp_10.showBit(flipCounting, 0); // sd write
    dsp_10.showBit(status->sdCardStatus == SD_STATUS_OFF, 1); // off
    dsp_10.showBit(status->sdCardStatus == SD_STATUS_ACTIVE, 2); // ativo
    dsp_10.showBit(status->sdCardStatus == SD_STATUS_READY, 3); // pronto
    

    // SD_STATUS_ERROR_NO_WRITE , SD_STATUS_ERROR_NO_SPACE , SD_STATUS_ERROR_NO_CARD
    // SD_STATUS_READY SD_STATUS_ACTIVE SD_STATUS_OFF

    dsp_11.showBit(status->sdCardStatus == SD_STATUS_ERROR_NO_WRITE, 0); // r-only
    dsp_11.showBit(status->sdCardStatus == SD_STATUS_ERROR_NO_SPACE, 1); // full
    dsp_11.showBit(status->sdCardStatus == SD_STATUS_ERROR_NO_CARD, 2); // no-card
    dsp_11.showBit(status->sdCardStatus == SD_STATUS_ERROR_WRITE_FAIL, 3); // w-error
    
  }

  lastSdStatus = status->sdCardStatus;

  lcd.setCursor(380, 30);
  dsp_setFont(&lcd, font_12);
  lcd.printf("%3d", printLoop);

  showCommMonitor(status);
  lcd.endWrite();
 
}

void readButton(statuses* status)
{

  if (SerialDash.available()) {
    char c = SerialDash.read();
    if (c == 'D') {
      // dump_SdCard(status);
    }
  }

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
        telaIndex++;
        

        if (telaIndex > 4) telaIndex = 0;

        lcd.startWrite();
        if (telaIndex == 0) { 
          printTitulo("Speed Monitor 1");
          dsp_00.setBackground(dsp_blue, "RPM", dspSize::size9);
          dsp_01.setBackground(dsp_blue, "TPS", dspSize::size9);
          dsp_10.setBackground(dsp_blue, "MAP", dspSize::size9);
          dsp_11.setBackground(dsp_black, "CLT", dspSize::size9);
        }
        else if (telaIndex == 1) {
          printTitulo("Speed Monitor 2");
          dsp_00.setBackground(dsp_blue, "IAT", dspSize::size9);
          dsp_01.setBackground(dsp_blue, "O2", dspSize::size9);
          dsp_10.setBackground(dsp_blue, "DWELL", dspSize::size9);
          dsp_11.setBackground(dsp_blue, "ADV", dspSize::size9);
        }
        else if (telaIndex == 2) {
          printTitulo("Speed Monitor 3");
          dsp_00.setBackground(dsp_blue, "BAT", dspSize::size9);
          dsp_01.setBackground(dsp_blue, "PW1", dspSize::size9);
          dsp_10.setBackground(dsp_blue, "OIL", dspSize::size9);
          dsp_11.setBackground(dsp_blue, "FUEL", dspSize::size9);
        }
        else if (telaIndex == 3) {
          printTitulo("Speed Monitor 4");
          dsp_00.setBackground(dsp_blue, "GAMAE", dspSize::size9);
          dsp_01.setBackground(dsp_blue, "ACCEL", dspSize::size9);
          dsp_10.setBackground(dsp_blue, "G_WUE", dspSize::size9);
          dsp_11.setBackground(dsp_blue, "G_EGO", dspSize::size9);
        }
        else if (telaIndex == 4) {
          lastSdStatus = 255;
          printTitulo("Logs and Alerts");
          dsp_00.setBackground(dsp_gray, "", dspSize::size9);

          dsp_01.setBackground(dsp_black, "Engine", dspSize::size9);
          dsp_01.maskBit("running", "crank", "warmup", "sync");
          
          dsp_10.setBackground(dsp_blue, "Sd-Card", dspSize::size9);
          dsp_10.maskBit("process", "sd-off", "active", "ready");

          dsp_11.setBackground(dsp_blue, "Sd-Card", dspSize::size9);
          dsp_11.maskBit("r-only", "full", "no-card", "w-error");

   
        }
        lcd.endWrite();
         
      }
      else if (btnStartLog.pressed(cursorX, cursorY) == true) {
        if (telaIndex == 4) {
          status->requestLogChange = true;
        }
        
      }
      cursorX = 0;
      cursorY = 0;
    }
  }
}

