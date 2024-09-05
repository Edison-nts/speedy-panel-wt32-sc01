#include "debugger.h"


Debugger::Debugger() {
};

size_t Debugger::write(uint8_t utf8)
{
 #ifdef DEBUGGER_ON
    Serial.write(utf8);
 #endif
 return 1U;
}