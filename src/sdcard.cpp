#include <SPI.h>
#include "global.h"
#include "debugger.h"
#include "FS.h"
 
#include "RingBuf.h"
#include "SdFat.h"

Debugger debug;

#define ENABLE_DEDICATED_SPI 0
#define SD_FAT_TYPE 1

#if SD_FAT_TYPE == 0
SdFat sd;
typedef File file_t;

#elif SD_FAT_TYPE == 1
SdFat32 sd;
typedef File32 file_t;

#elif SD_FAT_TYPE == 2
SdExFat sd;
typedef ExFile file_t;

#elif SD_FAT_TYPE == 3
SdFs sd;
typedef FsFile file_t;

#elif SD_FAT_TYPE == 99
#define sd SD
typedef File file_t;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

#define SD_SECTOR_SIZE    512 // Standard SD sector size
#define SD_LOG_ENTRY_SIZE 127    /**< The size of the live data packet used by the SD card.*/
// #define RING_BUF_CAPACITY (SD_LOG_ENTRY_SIZE * 10) //Allow for 10 entries in the ringbuffer. Will need tuning
#define RING_BUF_CAPACITY (SD_SECTOR_SIZE * 4U) // o dobro de 1 block
#define SD_LOG_FILE_SIZE  1024 * 1024 * 10 //Default 10mb file size

#define SD_CS_PIN 41

#define REASSIGN_PINS
static int sck = 39;
static int miso = 38;
static int mosi = 40;
static int cs = SD_CS_PIN;

// Max SPI rate for AVR is 10 MHz for F_CPU 20 MHz, 8 MHz for F_CPU 16 MHz.
#define SPI_CLOCK SD_SCK_MHZ(10)

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
// #define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
 
// static File * file; 
static char path[32] = "/speed_log.msl";
static uint32_t pathIndex = 0;

 
static file_t logFile;
static RingBuf<file_t, RING_BUF_CAPACITY> rb;
static uint32_t logStartTime;

// #define O_RDONLY 0X00  ///< Open for reading only.
// #define O_WRONLY 0X01  ///< Open for writing only.
// #define O_RDWR 0X02    ///< Open for reading and writing.
// #define O_AT_END 0X04  ///< Open at EOF.
// #define O_APPEND 0X08  ///< Set append mode.
// #define O_CREAT 0x10   ///< Create file if it does not exist.
// #define O_TRUNC 0x20   ///< Truncate file to zero length.
// #define O_EXCL 0x40    ///< Fail if the file exists.
// #define O_SYNC 0x80    ///< Synchronized write I/O operations.

static const char * header = R"(speeduino 202402: Speeduino 2024.02.1
Capture Date: Fri Aug 16 15:34:10 BRT 2024
Time	SecL	RPM	MAP	MAPxRPM	TPS	AFR	Lambda	IAT	CLT	Engine	DFCO	Gego	Gair	Gbattery	Gwarm	Gbaro	Gammae	Accel Enrich	VE (Current	VE1	VE2	PW	PW2	PW3	PW4	AFR Target	Lambda Target	Duty Cycle	Duty Cycle (Staging	TPS DOT	Advance (Current	Dwell	Dwell (Measured	Battery V	rpm/s	Error #	Error ID	Boost PSI	Boost cut	Hard Launch	Hard Limiter	Idle Control	IAC value	Idle Target RPM	Idle RPM Delta	Baro Pressure	Fan	Sync Loss #	Wheel Speed (kph	Wheel Speed (mph	Gear	Fuel Pressure	Oil Pressure	Loops/s	Loops/rev	Aux0	Aux1	Aux2	Aux3	Aux4	Aux5	Aux6	Aux7	Aux8	Aux9	Aux10	Aux11	Aux12	Aux13	Aux14	Aux15	Advance 1	Advance 2	FuelLoad	IgnitionLoad	Sync status	Engine Prot. RPM	Engine Prot. CLT	Trip Meter Miles	Odometer Miles	Vehicle Speed	Power	Torque
s	sec	rpm	kpa		%	O2				bits		%	%		%	%	%	%	%	%	%	ms	ms	ms	ms	O2				%/s	deg	ms	ms	V	rpm/s								Duty Cycle	RPM		kpa			km/h			PSI	PSI	loops																		deg	deg	kPa	kPa		On/Off	On/Off	Miles	Miles	MPH	HP	lbft
)";

void beginSDLogging(statuses* status)
{

   if(status->sdCardStatus != SD_STATUS_READY) {
        debug.println("beginSDLogging - sdcard not ready");
        return;
   }

   status->sdCardStatus = SD_STATUS_OFF;

    sprintf(path, "/speed_log_%d.msl", pathIndex);
    pathIndex++;

    debug.println("beginSDLogging - iniciando start log");
    delay(100);

    debug.println("beginSDLogging - testarndo se arquivo existe");
    delay(100);

    // sd.remove(path);
    // delay(100);
    // debug.println("beginSDLogging - remove ok");
    bool retOpen = false;

    retOpen = logFile.open(path, O_WRONLY | O_CREAT | O_TRUNC);
    

    debug.printf("beginSDLogging - Open %s \r\n", retOpen ? "OK" : "ERROR");
    delay(100);

    // file = nullptr;
    // logFile.close();
    if (!retOpen) {
      debug.println(path);
      debug.println("beginSDLogging - sdcard not open for write");
      // debug.printf("beginSDLogging - sdcard not open for write - errorCode (hex): %04x , errorData (hex): %04x", int(sd.card()->errorCode()), int(sd.card()->errorData()));
      status->sdCardStatus = SD_STATUS_ERROR_NO_WRITE;
      return;
    } 

    retOpen = logFile.preAllocate(SD_LOG_FILE_SIZE);
    debug.printf("beginSDLogging - preAllocate %s \r\n", retOpen ? "OK" : "ERROR");
    if (!retOpen) 
    {
      delay(100);
      debug.println("beginSDLogging - sdcard not responding for allocate");
      status->sdCardStatus = SD_STATUS_ERROR_NO_SPACE;
      return;
    }
  
    debug.println("beginSDLogging - log started");
    status->sdCardblockCount = 0U;
    rb.print(header);
    rb.sync();

    delay(100);
    debug.println("beginSDLogging - header printted");
    logStartTime = millis();
    status->sdCardStatus = SD_STATUS_ACTIVE;
    
}

void endSDLogging(statuses* status) 
{
     
    if(status->sdCardStatus == SD_STATUS_ACTIVE)
    {
        // Write any RingBuf data to logFile.
        rb.sync();
        
        logFile.truncate();
        logFile.rewind();
        
        logFile.close();

        logFile.sync(); //This is required to update the sd object. Without this any subsequent logfiles will overwrite this one
        
        status->sdCardStatus = SD_STATUS_READY;
        status->sdCardblockCount = 0U;
    }

}

void writeSDLogEntry(statuses* status, uint32_t timeMs)
{
    if(status->sdCardStatus != SD_STATUS_ACTIVE) {
        return;
    }

    double k = 1.00;
    double unk = 0.00;
    
        rb.printf("%6.3f\t", (timeMs - logStartTime) / 1000.00);    // Time					0.000	
        rb.printf("%d\t", status->secl);    // SecL					217	
        rb.printf("%d\t", status->RPM);    // RPM					995	
        rb.printf("%d\t", status->MAP);    // MAP					51	
        rb.printf("%d\t", unk);    // MAPxRPM				50745	
        rb.printf("%4.1f\t", status->TPS / 2.0);    // TPS					0.0	
        rb.printf("%6.3f\t", status->O2 / 10.0);    // AFR					12.800	
        rb.printf("%6.3f\t", status->O2 / (10.0 * 14.7));    // Lambda				0.871	
        rb.printf("%d\t", status->IAT);    // IAT					26	
        rb.printf("%d\t", status->coolant);    // CLT					42	
        rb.printf("%d\t", status->engine);    // Engine				9	
        rb.printf("%d\t", unk);    // DFCO					0	
        rb.printf("%d\t", status->egoCorrection);    // Gego					100	
        rb.printf("%d\t", status->iatCorrection);    // Gair					106	
        rb.printf("%d\t", status->batCorrection);    // Gbattery				100	
        rb.printf("%d\t", status->wueCorrection);    // Gwarm				109	
        rb.printf("%d\t", status->baroCorrection);    // Gbaro				100	
        rb.printf("%d\t", status->corrections);    // Gammae				116	
        rb.printf("%d\t", status->AEamount);    // Accel Enrich			100	
        rb.printf("%d\t", status->VE);    // VE Current			28	
        rb.printf("%d\t", status->VE1);    // VE1					28	
        rb.printf("%d\t", status->VE2);    // VE2					32	
        rb.printf("%6.3f\t", status->PW1 / 1000.0);    // PW1					5.558	
        rb.printf("%6.3f\t", status->PW2 / 1000.0);    // PW2					5.558	
        rb.printf("%6.3f\t", status->PW3 / 1000.0);    // PW3					0.000	
        rb.printf("%6.3f\t", status->PW4 / 1000.0);    // PW4					0.000	
        rb.printf("%6.3f\t", status->afrTarget / 10.0);    // AFR Target			14.000	
        rb.printf("%6.3f\t", status->afrTarget / (10.0 * 14.7));    // Lambda Target		0.952	
        rb.printf("%4.1f\t", status->idleLoad);    // Duty Cycle			4.6	 (idle)
        rb.printf("%4.1f\t", unk);    // Duty Cycle Staging	0.0	 -- { rpm && stagingEnabled ? ( 100.0*pulseWidth3/pulseLimit ) : 0
        rb.printf("%d\t", status->tpsDOT);    // TPS DOT				0	
        rb.printf("%d\t", status->advance);    // Advance Current		19	
        rb.printf("%6.3f\t", status->dwell / 1000.00);    // Dwell				5.580	
        rb.printf("%6.3f\t", status->actualDwell / 1000.00);    // Dwell Measured		5.389	
        rb.printf("%4.1f\t", status->battery10 / 10.00);    // Battery V			11.1	
        rb.printf("%d\t", status->rpmDOT);    // rpm/s				-20	
        rb.printf("%d\t", status->nextError && 0xF0);    // Error #				0	bits 0-3
        rb.printf("%d\t", status->nextError && 0x0F);    // Error ID				0	bits 4-7
        rb.printf("%d\t", unk);    // Boost PSI			-7.0	
        rb.printf("%d\t", unk);    // Boost cut			0	
        rb.printf("%d\t", BIT_CHECK(status->spark, 0));    // Hard Launch			0	
        rb.printf("%d\t", BIT_CHECK(status->spark, 2));    // Hard Limiter			0	
        rb.printf("%d\t", BIT_CHECK(status->spark, 6));    // Idle Control			1	
        rb.printf("%d\t", unk);    // IAC value			70	
        rb.printf("%d\t", status->CLIdleTarget);    // Idle Target RPM		1080	
        rb.printf("%d\t", unk);    // Idle RPM Delta		85	
        rb.printf("%d\t", status->baro);    // Baro Pressure		99	
        rb.printf("%d\t", BIT_CHECK(status->status4, 3));    // Fan					0	
        rb.printf("%d\t", status->syncLossCounter);    // Sync Loss #			4	
        rb.printf("%d\t", status->vss * k);    // Wheel Speed kph		0	
        rb.printf("%d\t", status->vss * k);    // Wheel Speed mph		0	
        rb.printf("%d\t", status->gear);    // Gear					0	
        rb.printf("%d\t", status->fuelPressure);    // Fuel Pressure		46	
        rb.printf("%d\t", status->oilPressure);    // Oil Pressure			46	
        rb.printf("%d\t", status->loopsPerSecond);    // Loops/s				1561	
        rb.printf("%6.2f\t", status->loopsPerSecond * k);    // Loops/rev			94.13	       
        rb.printf("%d\t", status->canin[0]);    // Aux0					579	
        rb.printf("%d\t", status->canin[1]);    // Aux1					1	
        rb.printf("%d\t", status->canin[2]);    // Aux2					1	
        rb.printf("%d\t", status->canin[3]);    // Aux3					1	
        rb.printf("%d\t", status->canin[4]);    // Aux4					1	
        rb.printf("%d\t", status->canin[5]);    // Aux5					1	
        rb.printf("%d\t", status->canin[6]);    // Aux6					0	
        rb.printf("%d\t", status->canin[7]);    // Aux7					1	
        rb.printf("%d\t", status->canin[8]);    // Aux8					1	
        rb.printf("%d\t", status->canin[9]);    // Aux9					1	
        rb.printf("%d\t", status->canin[10]);    // Aux10				0	
        rb.printf("%d\t", status->canin[11]);    // Aux11				1	
        rb.printf("%d\t", status->canin[12]);    // Aux12				0	
        rb.printf("%d\t", status->canin[13]);    // Aux13				1	
        rb.printf("%d\t", status->canin[14]);    // Aux14				0	
        rb.printf("%d\t", status->canin[15]);    // Aux15				1	
        
        rb.printf("%d\t", status->advance1);    // Advance 1			19	
        rb.printf("%d\t", status->advance2);    // Advance 2			0	
        rb.printf("%4.1f\t", status->fuelLoad);    // FuelLoad				51.0	
        rb.printf("%4.1f\t", status->ignLoad);    // IgnitionLoad			51.0	
        rb.printf("%d\t", BIT_CHECK(status->spark, 7));    // Sync status			2	
        
        rb.printf("%s\t", BIT_CHECK(status->engineProtectStatus, 0) ? "On": "Off");    // Engine Prot. RPM		Off	
        rb.printf("%s\t", BIT_CHECK(status->engineProtectStatus, 1) ? "On": "Off");    // Engine Prot. CLT		Off	
        
        rb.printf("%4.1f\t", 0.00);    // Trip Meter Miles		0.00	
        rb.printf("%4.1f\t", 0.00);    // Odometer Miles		0.00	
        rb.printf("%4.1f\t", status->vss);    // Vehicle Speed		0.00	
        rb.printf("%4.1f\t", 0.00);    // Power				0.0	
        rb.printf("%4.1f\t", 0.00);    // Torque				0.0
        rb.println();

        rb.sync();

        // debug.printf("writeSDLogEntry - entry: %d , time: %d ms \r\n", status->sdCardblockCount, (timeMs - logStartTime));

        status->sdCardblockCount++;

        // Serial.printf(" ... writeSDLogEntry block: %d \r\n", status->sdCardblockCount);

        //Check whether the file is full (IE When there is not enough room to write 1 more sector)
        // if( (logFile.dataLength() - logFile.curPosition()) < SD_SECTOR_SIZE)
        if (status->sdCardblockCount == 0)
        {
            debug.println("writeSDLogEntry - file sync process");
             // rb.sync();
            // logFile.sync(); 
        }

}

void sdcardSetup(statuses* status)
{
  status->sdCardblockCount = 0U;
  status->sdCardStatus = SD_STATUS_READY;
  rb.begin(&logFile);
  
  SPI.begin(sck, miso, mosi, cs);
  SPI.setDataMode(SPI_MODE0);

  delay(100);
  if (!sd.begin(SD_CONFIG))
  {
 
    debug.printf("sdcardSetup - NO CARD - errorCode (hex): %04x , errorData (hex): %04x", int(sd.card()->errorCode()), int(sd.card()->errorData()));
 
    
    // debug.println("sdcardSetup - NO CARD"); 
    status->sdCardStatus = SD_STATUS_ERROR_NO_CARD;
    return;
  }

  for (uint32_t i = 0; i < 256; i++) {
      sprintf(path, "/speed_log_%d.msl", i);
      if (sd.exists(path)) {
        if (i >= 255) {
          pathIndex = 0;
        } 
        else {
          pathIndex = i + 1;
        }
      }
      else {
        break;
      }
  }

  debug.printf("sdcardSetup - OK \r\n");
  

}
