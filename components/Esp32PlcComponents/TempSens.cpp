#include "Arduino.h"
#include "TempSens.hpp"

TempSens::TempSens(const String &n) : Base(n) 
{
}

void TempSens::setValue(float v)
{
    if(v!=value) 
    {
        changed=true;
        value=v;
    }

}

void TempSens::begin(uint64_t _addr, void (*_onChange)())
{
    addr=_addr;
    Base::begin(_onChange);
}

