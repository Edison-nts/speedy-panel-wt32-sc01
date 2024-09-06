#include <Arduino.h>
#include <EEPROM.h>
#include "FastCRC/FastCRC.h"


#ifndef GLOBALS_H

#define GLOBALS_H

// define the number of bytes you want to access
#define EEPROM_SIZE 1

#define CALIBRATION_TEMPERATURE_OFFSET 40
#define SerialEcu Serial0 // <- Serial Debug
#define SerialDash Serial // <- Serial USB

#define SD_STATUS_OFF               0 /**< SD system is inactive. FS and file remain closed */
#define SD_STATUS_READY             1 /**< Card is present and ready, but a log session has not commenced */
#define SD_STATUS_ACTIVE            2 /**< Log session commenced */
#define SD_STATUS_ERROR_NO_CARD     3 /**< No SD card found when attempting to open file */
#define SD_STATUS_ERROR_NO_FS       4 /**< No filesystem found when attempting to open file */
#define SD_STATUS_ERROR_NO_WRITE    5 /**< Card and filesystem found, however file creation failed due to no write access */
#define SD_STATUS_ERROR_NO_SPACE    6 /**< File could not be preallocated as there is not enough space on card */
#define SD_STATUS_ERROR_WRITE_FAIL  7 /**< Log file created and opened, but a sector write failed during logging */
#define SD_STATUS_ERROR_FORMAT_FAIL 8 /**< Attempted formatting of SD card failed */

#define SD_STATUS_CARD_PRESENT      0 //0=no card, 1=card present
#define SD_STATUS_CARD_TYPE         1 //0=SD, 1=SDHC
#define SD_STATUS_CARD_READY        2 //0=not ready, 1=ready
#define SD_STATUS_CARD_LOGGING      3 //0=not logging, 1=logging
#define SD_STATUS_CARD_ERROR        4 //0=no error, 1=error
#define SD_STATUS_CARD_VERSION      5 //0=1.x, 1=2.x
#define SD_STATUS_CARD_FS           6 //0=no FAT16, 1=FAT32
#define SD_STATUS_CARD_UNUSED       7 //0=normal, 1=unused

// Handy bitsetting macros
//  var -> o byte , pos -> a posicao de 0 - 15
#define BIT_SET(var, pos) ((var) |= (1U << (pos)))
#define BIT_CLEAR(var, pos) ((var) &= ~(1U << (pos)))
#define BIT_CHECK(var, pos) !!((var) & (1U << (pos)))
#define BIT_TOGGLE(var, pos) ((var) ^= (1UL << (pos)))
#define BIT_WRITE(var, pos, bitvalue) ((bitvalue) ? BIT_SET((var), (pos)) : bitClear((var), (pos)))

#ifndef word
#define word(h, l) ((h << 8) | l) // recebe hi e lo , retorna uint16_t
#endif

struct statuses {
  volatile bool hasSync : 1; /**< Flag for crank/cam position being known by decoders (See decoders.ino).
  This is used for sanity checking e.g. before logging tooth history or reading some sensors and computing readings. */
  bool initialisationComplete : 1; //Tracks whether the setup() function has run completely
  bool clutchTrigger : 1;
  bool previousClutchTrigger : 1;
  volatile bool fpPrimed : 1; //Tracks whether or not the fuel pump priming has been completed yet
  volatile bool injPrimed : 1; //Tracks whether or not the injector priming has been completed yet
  volatile bool tachoSweepEnabled : 1;
  volatile bool tachoAlt : 1;
    
  uint16_t RPM;   ///< RPM - Current Revs per minute
  byte RPMdiv100; ///< RPM value scaled (divided by 100) to fit a byte (0-255, e.g. 12000 => 120)
  long longRPM;   ///< RPM as long int (gets assigned to / maintained in statuses.RPM as well)
  uint16_t mapADC;
  int baroADC;
  long MAP;     ///< Manifold absolute pressure. Has to be a long for PID calcs (Boost control)
  int16_t EMAP; ///< EMAP ... (See @ref config6.useEMAP for EMAP enablement)
  uint16_t EMAPADC;
  byte baro;   ///< Barometric pressure is simply the initial MAP reading, taken before the engine is running. Alternatively, can be taken from an external sensor
  byte TPS;    /**< The current TPS reading (0% - 100%). Is the tpsADC value after the calibration is applied */
  byte tpsADC; /**< byte (valued: 0-255) representation of the TPS. Downsampled from the original 10-bit (0-1023) reading, but before any calibration is applied */
  int16_t tpsDOT; /**< TPS delta over time. Measures the % per second that the TPS is changing. Note that is signed value, because TPSdot can be also negative */
  byte TPSlast; /**< The previous TPS reading */
  int16_t mapDOT; /**< MAP delta over time. Measures the kpa per second that the MAP is changing. Note that is signed value, because MAPdot can be also negative */
  volatile int rpmDOT; /**< RPM delta over time (RPM increase / s ?) */
  byte VE;     /**< The current VE value being used in the fuel calculation. Can be the same as VE1 or VE2, or a calculated value of both. */
  byte VE1;    /**< The VE value from fuel table 1 */
  byte VE2;    /**< The VE value from fuel table 2, if in use (and required conditions are met) */
  byte O2;     /**< Primary O2 sensor reading */
  byte O2_2;   /**< Secondary O2 sensor reading */
  int coolant; /**< Coolant temperature reading */
  int cltADC;
  int IAT;     /**< Inlet air temperature reading */
  int iatADC;
  int batADC;
  int O2ADC;
  int O2_2ADC;
  int dwell;          ///< dwell (coil primary winding/circuit on) time (in ms * 10 ? See @ref correctionsDwell)
  volatile int16_t actualDwell;    ///< actual dwell time if new ignition mode is used (in uS)
  byte dwellCorrection; /**< The amount of correction being applied to the dwell time (in unit ...). */
  byte battery10;     /**< The current BRV in volts (multiplied by 10. Eg 12.5V = 125) */
  int8_t advance;     /**< The current advance value being used in the spark calculation. Can be the same as advance1 or advance2, or a calculated value of both */
  int8_t advance1;    /**< The advance value from ignition table 1 */
  int8_t advance2;    /**< The advance value from ignition table 2 */
  uint16_t corrections; /**< The total current corrections % amount */
  uint16_t AEamount;    /**< The amount of acceleration enrichment currently being applied. 100=No change. Varies above 255 */
  byte egoCorrection; /**< The amount of closed loop AFR enrichment currently being applied */
  byte wueCorrection; /**< The amount of warmup enrichment currently being applied */
  byte batCorrection; /**< The amount of battery voltage enrichment currently being applied */
  byte iatCorrection; /**< The amount of inlet air temperature adjustment currently being applied */
  byte baroCorrection; /**< The amount of correction being applied for the current baro reading */
  byte launchCorrection;   /**< The amount of correction being applied if launch control is active */
  byte flexCorrection;     /**< Amount of correction being applied to compensate for ethanol content */
  byte fuelTempCorrection; /**< Amount of correction being applied to compensate for fuel temperature */
  int8_t flexIgnCorrection;/**< Amount of additional advance being applied based on flex. Note the type as this allows for negative values */
  byte afrTarget;    /**< Current AFR Target looked up from AFR target table (x10 ? See @ref afrTable)*/
  byte CLIdleTarget; /**< The target idle RPM (when closed loop idle control is active) */
  bool idleUpActive; /**< Whether the externally controlled idle up is currently active */
  bool CTPSActive;   /**< Whether the externally controlled closed throttle position sensor is currently active */
  volatile byte ethanolPct; /**< Ethanol reading (if enabled). 0 = No ethanol, 100 = pure ethanol. Eg E85 = 85. */
  volatile int8_t fuelTemp;
  unsigned long AEEndTime; /**< The target end time used whenever AE (acceleration enrichment) is turned on */
  volatile byte status1; ///< Status bits (See BIT_STATUS1_* defines on top of this file)
  volatile byte spark;   ///< Spark status/control indicator bits (launch control, boost cut, spark errors, See BIT_SPARK_* defines)
  volatile byte spark2;  ///< Spark 2 ... (See also @ref config10 spark2* members and BIT_SPARK2_* defines)
  uint8_t engine; ///< Engine status bits (See BIT_ENGINE_* defines on top of this file)
  unsigned int PW1; ///< In uS
  unsigned int PW2; ///< In uS
  unsigned int PW3; ///< In uS
  unsigned int PW4; ///< In uS
  unsigned int PW5; ///< In uS
  unsigned int PW6; ///< In uS
  unsigned int PW7; ///< In uS
  unsigned int PW8; ///< In uS
  volatile byte runSecs; /**< Counter of seconds since cranking commenced (Maxes out at 255 to prevent overflow) */
  volatile byte secl; /**< Counter incrementing once per second. Will overflow after 255 and begin again. This is used by TunerStudio to maintain comms sync */
  volatile uint32_t loopsPerSecond; /**< A performance indicator showing the number of main loops that are being executed each second */ 
  bool launchingSoft; /**< Indicator showing whether soft launch control adjustments are active */
  bool launchingHard; /**< Indicator showing whether hard launch control adjustments are active */
  uint16_t freeRAM;
  unsigned int clutchEngagedRPM; /**< The RPM at which the clutch was last depressed. Used for distinguishing between launch control and flat shift */ 
  bool flatShiftingHard;
  volatile uint32_t startRevolutions; /**< A counter for how many revolutions have been completed since sync was achieved. */
  uint16_t boostTarget;
  byte testOutputs;   ///< Test Output bits (only first bit used/tested ?)
  bool testActive;    // Not in use ? Replaced by testOutputs ?
  uint16_t boostDuty; ///< Boost Duty percentage value * 100 to give 2 points of precision
  byte idleLoad;      ///< Either the current steps or current duty cycle for the idle control
  uint16_t canin[16]; ///< 16bit raw value of selected canin data for channels 0-15
  uint8_t current_caninchannel = 0; /**< Current CAN channel, defaults to 0 */
  uint16_t crankRPM = 400; /**< The actual cranking RPM limit. This is derived from the value in the config page, but saves us multiplying it every time it's used (Config page value is stored divided by 10) */
  volatile byte status3; ///< Status bits (See BIT_STATUS3_* defines on top of this file)
  int16_t flexBoostCorrection; /**< Amount of boost added based on flex */
  byte nitrous_status;
  byte nSquirts;  ///< Number of injector squirts per cycle (per injector)
  byte nChannels; /**< Number of fuel and ignition channels.  */
  int16_t fuelLoad;
  int16_t fuelLoad2;
  int16_t ignLoad;
  int16_t ignLoad2;
  bool fuelPumpOn; /**< Indicator showing the current status of the fuel pump */
  volatile byte syncLossCounter;
  byte knockRetard;
  bool knockActive;
  bool toothLogEnabled;
  byte compositeTriggerUsed; // 0 means composite logger disabled, 2 means use secondary input (1st cam), 3 means use tertiary input (2nd cam), 4 means log both cams together
  int16_t vvt1Angle; //Has to be a long for PID calcs (CL VVT control)
  byte vvt1TargetAngle;
  long vvt1Duty; //Has to be a long for PID calcs (CL VVT control)
  uint16_t injAngle;
  byte ASEValue;
  uint16_t vss;      /**< Current speed reading. Natively stored in kph and converted to mph in TS if required */
  bool idleUpOutputActive; /**< Whether the idle up output is currently active */
  byte gear;         /**< Current gear (Calculated from vss) */
  byte fuelPressure; /**< Fuel pressure in PSI */
  byte oilPressure;  /**< Oil pressure in PSI */
  byte engineProtectStatus;
  byte fanDuty;
  byte wmiPW;
  volatile byte status4; ///< Status bits (See BIT_STATUS4_* defines on top of this file)
  int16_t vvt2Angle; //Has to be a long for PID calcs (CL VVT control)
  byte vvt2TargetAngle;
  long vvt2Duty; //Has to be a long for PID calcs (CL VVT control)
  byte outputsStatus;
  byte TS_SD_Status; //TunerStudios SD card status
  byte airConStatus;
  byte nextError;
  volatile uint8_t receiveCount;
  volatile uint8_t transmitCount;
  volatile bool currentStatusChanged;
  volatile bool requestLogChange;
  uint8_t sdCardStatus;
  volatile bool requestLogStopWrite;
  volatile uint16_t sdCardblockCount;
};

  
static FastCRC32 CRC32_serial;
 

static __attribute__((noinline)) uint32_t reverse_bytes(uint32_t i)
{
  return ((i >> 24) & 0xffU) |    // move byte 3 to byte 0
         ((i << 8) & 0xff0000U) | // move byte 1 to byte 2
         ((i >> 8) & 0xff00U) |   // move byte 2 to byte 1
         ((i << 24) & 0xff000000U);
}

// ecu.cpp
void enviarRequestRead(statuses* status, uint8_t blockNum);
void readSerialEcu(statuses* status);

// display.cpp
void displaySetup(statuses* status);
void readButton(statuses* status);
void showData(statuses* status);

void showCommMonitor(statuses* status);
void printTitulo(const char *linha);

// sdcard.cpp
void beginSDLogging(statuses* status);
void endSDLogging(statuses* status);
void writeSDLogEntry(statuses* status, uint32_t time);
void sdcardSetup(statuses* status);
void initFile(const char *message);
// void dump_SdCard(statuses* status);

#endif
