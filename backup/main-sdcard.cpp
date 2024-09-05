#include <Arduino.h>

// micro sd
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define REASSIGN_PINS
static int sck = 39;
static int miso = 38;
static int mosi = 40;
static int cs = 41;
static uint32_t n = 0;

void appendFile(const char *path, const char *message);
void writeFile(const char *path, const char *message);
void readFile(const char *path);
void listDir(const char *dirname, uint8_t levels);
void sdcardSetup(void *args);

void setup(void) {
  Serial.begin(115200);
  delay(10000);
  Serial.println(" ... inicio ");
  sdcardSetup(null);
}

void loop() {
  n++;
  char buffer[255];
  sprintf(buffer, "linha %d \r\n", n);
  appendFile("/hello.txt", buffer);
  Serial.println();
  Serial.println(" ----------------  ");
  readFile("/hello.txt");
  delay(1000);

}

void listDir(const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = SD.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(file.path(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void readFile(const char *path)
{
  Serial.printf("Reading file: %s\n", path);

  File file = SD.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available())
  {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = SD.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = SD.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}

void sdcardSetup(void *args)
{
  SPI.begin(sck, miso, mosi, cs);
  SPI.setDataMode(SPI_MODE0);
  delay(100);
  if (!SD.begin(cs))
  {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  // Serial.print("SD Card Type: ");
  // if (cardType == CARD_MMC)
  // {
  //   Serial.println("MMC");
  // }
  // else if (cardType == CARD_SD)
  // {
  //   Serial.println("SDSC");
  // }
  // else if (cardType == CARD_SDHC)
  // {
  //   Serial.println("SDHC");
  // }
  // else
  // {
  //   Serial.println("UNKNOWN");
  // }

  // uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  // Serial.printf("SD Card Size: %lluMB\n", cardSize);

  // listDir("/", 0);
  writeFile("/hello.txt", "Hello ");
  // appendFile("/hello.txt", "World!\n");
  // readFile("/hello.txt");
  // listDir("/", 0);
}
