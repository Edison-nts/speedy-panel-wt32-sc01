#include <Arduino.h>
#include <Print.h>

#ifndef _DEBUGGER

#define _DEBUGGER

// comenta para nao debugar na serial
// #define DEBUGGER_ON

class Debugger : public Print
{

    public:
    Debugger();
    size_t write(uint8_t utf8);

};

#endif