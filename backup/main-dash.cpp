#include "global.h"

static struct statuses currentStatus;

void setup(void)
{
  EEPROM.begin(EEPROM_SIZE);
  uint8_t emuState = EEPROM.read(0);

  SerialDash.begin(115200); // <- USB
  SerialEcu.begin(115200);  // <- debug pins rx e tx
  delay(2000);

  currentStatus.currentStatusChanged = true;
  currentStatus.receiveCount = 0;
  currentStatus.transmitCount = 2;
   
  displaySetup(&currentStatus);

  sdcardSetup(&currentStatus);

  xTaskCreatePinnedToCore(comm_loop,
    "comm_loop",
    4096,          // usStackDepth
    NULL, // pvParameters
    4,    // uxPriority
    NULL,          // pvCreatedTask
    1);  // xCoreID

	xTaskCreatePinnedToCore(display_loop,
    "display_loop",
    4096,			// usStackDepth
    NULL, // pvParameters
    3,				// uxPriority
    NULL,			// pvCreatedTask
    0);
  
  // uint32_t tempo = CRC32_serial.crc32(buffer, length);
}

void loop() {}

void display_loop(void *args) {
  uint32_t delay = 10; // delay padrao 10ms
  static int8_t nLoop = 0;
  vTaskDelay(100);
  
  while(true) {
    delay = 10;
    readButton(&currentStatus);

    if (currentStatus.currentStatusChanged) {
      delay = 50;
      nLoop = 0;
      showData(&currentStatus);
      // writeSDLogEntry(&currentStatus, millis());
    }
    else if (nLoop % 8 == 0) {
      delay = 50;
      showData(&currentStatus);
      // writeSDLogEntry(&currentStatus, millis());
    }
    nLoop++;

    if (currentStatus.requestLogChange) {
      currentStatus.requestLogChange = false;
      if (currentStatus.sdCardStatus == SD_STATUS_ACTIVE) {
        endSDLogging(&currentStatus);
      }
      else if (currentStatus.sdCardStatus == SD_STATUS_READY) {
        beginSDLogging(&currentStatus);
      }

    }
    
    vTaskDelay(delay);
  }

}

void comm_loop(void *args) {
  uint32_t delay = 12; // delay padrao 12ms
  uint8_t nLoop = 0;
  vTaskDelay(100);
  
  while(true) {
    if (SerialEcu.available())
    {
      readSerialEcu(&currentStatus);
      nLoop = 0;
    }
    nLoop++;

    if (nLoop % 4 == 0) 
    {
      enviarRequestRead(&currentStatus, 0);
    }
    else if (nLoop % 4 == 1) {
      writeSDLogEntry(&currentStatus, millis());
    }

    vTaskDelay(delay);
  }

}

