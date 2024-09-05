#include "global.h"

static uint8_t r_dataLen = 64;
static uint8_t r_offset = 0;
static uint8_t buffer[256];

void readSerialEcu_n (statuses* status, byte * buffer);
void readSerialEcu_r (statuses* status, byte * buffer);

void enviarRequestRead(statuses* status, uint8_t blockNum)
{ 
    status->transmitCount++;
    // SerialEcu.write('n');

    if (blockNum == 1) {
      r_offset = 64;
      r_dataLen = 127 - 64; // 63
    }
    else {
      r_offset = 0;
      r_dataLen = 64;
    }

    SerialEcu.write('r');
    SerialEcu.write(0x00);      // canid
    SerialEcu.write(0x30);      // Output
    SerialEcu.write(r_offset);  // offset low
    SerialEcu.write(0U);        // offset hi
    SerialEcu.write(r_dataLen); // length low
    SerialEcu.write(0U);        // length hi

    
}

void readSerialEcu(statuses* status)
{
  status->receiveCount++;
  unsigned long last = micros();
  uint8_t index = 0;
  char cmd = SerialEcu.read();
  uint16_t dataLen = 0;
  uint16_t offset = 0;

  if (cmd == 'r') { // comando r, header com 2 bytes -> r , 0x30 
    while(SerialEcu.available() < 1) {}
    SerialEcu.read(); // bypass 0x30 return command
    dataLen = r_dataLen;
    offset = r_offset;
  }
  else if (cmd == 'n') { // comando n , header 3 bytes -> n , 0x32 , len (123 bytes)
    while(SerialEcu.available() < 2) {}
    SerialEcu.read(); // bypass 0x32 return command
    dataLen = SerialEcu.read();

  }
  else {
    while(SerialEcu.available()) {
      SerialEcu.read();
    }
    return;
  }


  while (index < dataLen)
  {
    if (SerialEcu.available())
    {
      last = micros();
      buffer[index + offset] = SerialEcu.read();
      index++;
    }
    else if (micros() - last > 50)
    {
      break;
    }
  }

  // limpa a serial se tiver lixo
  if (SerialEcu.available()) {
    while(SerialEcu.available()) {
      SerialEcu.read();
    }
  }
  // SerialDash.printf(" ... dataLen %d , index %d, offset: %d \r\n", dataLen, index, offset);

  // if (index != dataLen) {
  //   SerialDash.printf(" ... qui porra , cade meus bytes %d , so chegou %d, offset: %d \r\n", dataLen, index, offset);
  //   return;
  // }

  if (cmd == 'r') {
    if (r_offset == 0) {
      enviarRequestRead(status, 1);
    }
    else {
      readSerialEcu_r(status, buffer);
    }
    
  }
  else if (cmd == 'n') {
    readSerialEcu_n(status, buffer);
  }
  
 
}

void readSerialEcu_n (statuses* status, byte * buffer) {
  

    status->secl                = buffer[0];
    status->status1             = buffer[1];
    status->engine              = buffer[2];
    status->dwell               = buffer[3] * 100; //Dwell in ms * 10 // (byte)div100(status->dwell)
    status->MAP                 = word(buffer[5],buffer[4]);
    status->IAT                 = buffer[6] - CALIBRATION_TEMPERATURE_OFFSET;
    status->coolant             = buffer[7] - CALIBRATION_TEMPERATURE_OFFSET;  
    status->batCorrection       = buffer[8]; 
    status->battery10           = buffer[9];
    status->O2                  = buffer[10]; 
    status->egoCorrection       = buffer[11]; 
    status->iatCorrection       = buffer[12]; 
    status->wueCorrection       = buffer[13]; //Warmup enrichment (%)
    status->RPM                 = word(buffer[15],buffer[14]); 
    status->AEamount            = buffer[16]; //acceleration enrichment (%)
    status->corrections         = buffer[17]; //Total GammaE (%)
    status->VE                  = buffer[18]; //Current VE 1 (%)
    status->afrTarget           = buffer[19]; 
    status->PW1                 = word(buffer[21],buffer[20]); //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
    status->tpsDOT              = buffer[22] * 10; //TPS DOT // (uint8_t)(status->tpsDOT / 10)
    status->advance             = buffer[23];
    status->TPS                 = buffer[24]; // TPS (0% to 100%)
    status->loopsPerSecond      = word(buffer[26],buffer[25]);  //(byte)((status->loopsPerSecond >> 8) & 0xFF);
    status->freeRAM             = word(buffer[28],buffer[27]); 
    status->boostTarget         = buffer[29] * 2; //Divide boost target by 2 to fit in a byte // (byte)(status->boostTarget >> 1)
    status->boostDuty           = buffer[30] * 100; // (byte)(status->boostDuty / 100)
    status->spark               = buffer[31]; //Spark related bitfield, launchHard(0), launchSoft(1), hardLimitOn(2), softLimitOn(3), boostCutSpark(4), error(5), idleControlOn(6), sync(7)
    status->rpmDOT              = word(buffer[33], buffer[32]);
    status->ethanolPct          = buffer[34]; //Flex sensor value (or 0 if not used)
    status->flexCorrection      = buffer[35]; //Flex fuel correction (% above or below 100)
    status->flexIgnCorrection   = buffer[36]; //Ignition correction (Increased degrees of advance) for flex fuel
    status->idleLoad            = buffer[37]; 
    status->testOutputs         = buffer[38]; // testEnabled(0), testActive(1)
    status->O2_2                = buffer[39]; //O2
    status->baro                = buffer[40]; //Barometer value
    status->canin[0]            = word(buffer[42], buffer[41]);
    status->canin[1]            = word(buffer[44], buffer[43]);
    status->canin[2]            = word(buffer[46], buffer[45]);
    status->canin[3]            = word(buffer[48], buffer[47]);
    status->canin[4]            = word(buffer[50], buffer[49]);
    status->canin[5]            = word(buffer[52], buffer[51]);
    status->canin[6]            = word(buffer[54], buffer[53]);
    status->canin[7]            = word(buffer[56], buffer[55]);
    status->canin[8]            = word(buffer[58], buffer[57]);
    status->canin[9]            = word(buffer[60], buffer[59]);
    status->canin[10]           = word(buffer[62], buffer[61]);
    status->canin[11]           = word(buffer[64], buffer[63]);
    status->canin[12]           = word(buffer[66], buffer[65]);
    status->canin[13]           = word(buffer[68], buffer[67]);
    status->canin[14]           = word(buffer[70], buffer[69]);
    status->canin[15]           = word(buffer[72], buffer[71]);
    status->tpsADC              = buffer[73];
    status->nextError           = buffer[74];
    status->launchCorrection    = buffer[75];
    status->PW2                 = word(buffer[77], buffer[76]); //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
    status->PW3                 = word(buffer[79], buffer[78]); //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
    status->PW4                 = word(buffer[81], buffer[80]);  //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.
    status->status3             = buffer[82];  // resentLockOn(0), nitrousOn(1), fuel2Active(2), vssRefresh(3), halfSync(4), nSquirts(6:7)
    status->engineProtectStatus = buffer[83]; //RPM(0), MAP(1), OIL(2), AFR(3), Unused(4:7)
    status->fuelLoad            = word(buffer[85], buffer[84]); 
    status->ignLoad             = word(buffer[87], buffer[86]);
    status->injAngle            = word(buffer[89], buffer[88]); 
    status->idleLoad            = buffer[90];
    status->CLIdleTarget        = buffer[91];//closed loop idle target
    status->mapDOT              = buffer[92] * 10;  //rate of change of the map // (uint8_t)(status->mapDOT / 10) 
    status->vvt1Angle           = buffer[93];  // (int8_t)status->vvt1Angle
    status->vvt1TargetAngle     = buffer[94]; 
    status->vvt1Duty            = buffer[95];
    status->flexBoostCorrection = word(buffer[97], buffer[96]);
    status->baroCorrection      = buffer[98];
    status->ASEValue            = buffer[99]; //Current ASE (%)
    status->vss                 = word(buffer[101], buffer[100]); //speed reading from the speed sensor
    status->gear                = buffer[102]; 
    status->fuelPressure        = buffer[103];
    status->oilPressure         = buffer[104];
    status->wmiPW               = buffer[105];
    status->status4             = buffer[106]; // wmiEmptyBit(0), vvt1Error(1), vvt2Error(2), fanStatus(3), UnusedBits(4:7)
    status->vvt2Angle           = buffer[107];
    status->vvt2TargetAngle     = buffer[108];
    status->vvt2Duty            = buffer[109]; 
    status->outputsStatus       = buffer[110]; 
    status->fuelTemp            = buffer[111] - CALIBRATION_TEMPERATURE_OFFSET; //Fuel temperature from flex sensor // (byte)(status->fuelTemp + CALIBRATION_TEMPERATURE_OFFSET)
    status->fuelTempCorrection  = buffer[112]; //Fuel temperature Correction (%)
    status->VE1                 = buffer[113]; //VE 1 (%)
    status->VE2                 = buffer[114]; //VE 2 (%)
    status->advance1            = buffer[115]; //advance 1 
    status->advance2            = buffer[116]; //advance 2 
    status->nitrous_status      = buffer[117];
    status->TS_SD_Status        = buffer[118];  //SD card status
    status->EMAP                = word(buffer[120], buffer[119]); //2 bytes for EMAP
    status->fanDuty             = buffer[121];
    status->airConStatus        = buffer[122]; 
    status->currentStatusChanged = true;
  
}

void readSerialEcu_r (statuses* status, byte * buffer) {
   

    status->secl =                  buffer[0]; //secl is simply a counter that increments each second. Used to track unexpected resets (Which will reset this count to 0)
    status->status1 =               buffer[1]; //status1 Bitfield
    status->engine =                buffer[2]; //Engine Status Bitfield
    status->syncLossCounter =       buffer[3];
    status->MAP =                   word(buffer[5], buffer[4]); //2 bytes for MAP
    status->IAT =                   buffer[6] - CALIBRATION_TEMPERATURE_OFFSET; //mat
    status->coolant =               buffer[7] - CALIBRATION_TEMPERATURE_OFFSET; //Coolant ADC
    status->batCorrection =         buffer[8]; //Battery voltage correction (%)
    status->battery10 =             buffer[9]; //battery voltage
    status->O2 =                    buffer[10]; //O2
    status->egoCorrection =         buffer[11]; //Exhaust gas correction (%)
    status->iatCorrection =         buffer[12]; //Air temperature Correction (%)
    status->wueCorrection =         buffer[13]; //Warmup enrichment (%)
    status->RPM =                   word(buffer[15], buffer[14]); //rpm HB
    status->AEamount =              buffer[16] * 2; //TPS acceleration enrichment (%) divided by 2 (Can exceed 255)
    status->corrections =           word(buffer[18], buffer[17]); //Total GammaE (%)
    status->VE1 =                   buffer[19]; //VE 1 (%)
    status->VE2 =                   buffer[20]; //VE 2 (%)
    status->afrTarget =             buffer[21];
    status->tpsDOT =                word(buffer[23], buffer[22]); //TPS DOT
    status->advance =               buffer[24];
    status->TPS =                   buffer[25]; // TPS (0% to 100%)
    status->loopsPerSecond =        word(buffer[27], buffer[26]);
    status->freeRAM =               word(buffer[29], buffer[28]);
    status->boostTarget =           buffer[30] * 2; //Divide boost target by 2 to fit in a byte
    status->boostDuty =             buffer[31] * 100;
    status->spark =                 buffer[32]; //Spark related bitfield
    status->rpmDOT =                word(buffer[34], buffer[33]); //rpmDOT must be sent as a signed integer
    status->ethanolPct =            buffer[35]; //Flex sensor value (or 0 if not used)
    status->flexCorrection =        buffer[36]; //Flex fuel correction (% above or below 100)
    status->flexIgnCorrection =     buffer[37]; //Ignition correction (Increased degrees of advance) for flex fuel
    status->idleLoad =              buffer[38];
    status->testOutputs =           buffer[39];
    status->O2_2 =                  buffer[40]; //O2
    status->baro =                  buffer[41]; //Barometer value
    status->canin[0] =              word(buffer[43], buffer[42]);
    status->canin[1] =              word(buffer[45], buffer[44]);
    status->canin[2] =              word(buffer[47], buffer[46]);
    status->canin[3] =              word(buffer[49], buffer[48]);
    status->canin[4] =              word(buffer[51], buffer[50]);
    status->canin[5] =              word(buffer[53], buffer[52]);
    status->canin[6] =              word(buffer[55], buffer[54]);
    status->canin[7] =              word(buffer[57], buffer[56]);
    status->canin[8] =              word(buffer[59], buffer[58]);
    status->canin[9] =              word(buffer[61], buffer[60]);
    status->canin[10] =             word(buffer[63], buffer[62]);
    status->canin[11] =             word(buffer[65], buffer[64]);
    status->canin[12] =             word(buffer[67], buffer[66]);
    status->canin[13] =             word(buffer[69], buffer[68]);
    status->canin[14] =             word(buffer[71], buffer[70]);
    status->canin[15] =             word(buffer[73], buffer[72]);
    status->tpsADC =                buffer[74];
    status->nextError =             buffer[75];
    status->PW1 =                   word(buffer[77], buffer[76]); //Pulsewidth 1 multiplied by 10 in ms. Have to convert from uS to mS.
    status->PW2 =                   word(buffer[79], buffer[78]); //Pulsewidth 2 multiplied by 10 in ms. Have to convert from uS to mS.
    status->PW3 =                   word(buffer[81], buffer[80]); //Pulsewidth 3 multiplied by 10 in ms. Have to convert from uS to mS.
    status->PW4 =                   word(buffer[83], buffer[82]); //Pulsewidth 4 multiplied by 10 in ms. Have to convert from uS to mS.
    status->status3 =               buffer[84];
    status->engineProtectStatus =   buffer[85];
    status->fuelLoad =              word(buffer[87], buffer[86]);
    status->ignLoad =               word(buffer[89], buffer[88]);
    status->dwell =                 word(buffer[91], buffer[90]);
    status->CLIdleTarget =          buffer[92];
    status->mapDOT =                word(buffer[94], buffer[93]);
    status->vvt1Angle =             word(buffer[96], buffer[95]); //2 bytes for vvt1Angle
    status->vvt1TargetAngle =       buffer[97];
    status->vvt1Duty =              buffer[98];
    status->flexBoostCorrection =   word(buffer[100], buffer[99]);
    status->baroCorrection =        buffer[101];
    status->VE =                    buffer[102]; //Current VE (%). Can be equal to VE1 or VE2 or a calculated value from both of them
    status->ASEValue =              buffer[103]; //Current ASE (%)
    status->vss =                   word(buffer[105], buffer[104]);
    status->gear =                  buffer[106];
    status->fuelPressure =          buffer[107];
    status->oilPressure =           buffer[108];
    status->wmiPW =                 buffer[109];
    status->status4 =               buffer[110];
    status->vvt2Angle =             word(buffer[112], buffer[111]); //2 bytes for vvt2Angle
    status->vvt2TargetAngle =       buffer[113];
    status->vvt2Duty =              buffer[114];
    status->outputsStatus =         buffer[115];
    status->fuelTemp =              buffer[116] - CALIBRATION_TEMPERATURE_OFFSET; //Fuel temperature from flex sensor
    status->fuelTempCorrection =    buffer[117]; //Fuel temperature Correction (%)
    status->advance1 =              buffer[118]; //advance 1 (%)
    status->advance2 =              buffer[119]; //advance 2 (%)
    status->TS_SD_Status =          buffer[120]; //SD card status
    status->EMAP =                  word(buffer[122], buffer[121]); //2 bytes for EMAP
    status->fanDuty =               buffer[123];
    status->airConStatus =          buffer[124];
    status->actualDwell =           word(buffer[126], buffer[125]);
    status->currentStatusChanged = true;
}  

