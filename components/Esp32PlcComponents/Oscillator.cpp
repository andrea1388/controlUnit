#include "Arduino.h"
#include "Oscillator.hpp"
void Oscillator::run()
{
    unsigned long m=millis();
    if(state)
    {
        if(!enabled)
        {
            state=false;
            changed=true;
        }
        else
        {
            if((m-tLastChange)>tOn)
            {
                state=false;
                changed=true;
                tLastChange=m;             
            }
        }
    }
    else
    {
        if((m-tLastChange)>tOff)
        {
            state=true;
            changed=true;
            tLastChange=m;             
        }
    }
    Base::run();    
}

void Oscillator::begin(uint32_t _tOn, uint32_t _tOff, void (*_onChange)(), bool _enabled)
{
    tOn=_tOn;
    tOff=_tOff;
    enabled=_enabled;
    Base::begin(_onChange);
}

Oscillator::Oscillator(const String& n) : Base(n)
{
}
