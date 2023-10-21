#include "Arduino.h"
#include "Oscillator.hpp"
void Oscillator::run()
{
    uint32_t m=millis();
    if(state)
    {
        if(!enabled)
        {
            state=false;
            if(onChange) onChange(state);
        }
        else
        {
            if((m-tLastChange)>tOn)
            {
                state=false;
                if(onChange) onChange(state);  
                tLastChange=m;             
            }
        }
    }
    else
    {
        if((m-tLastChange)>tOff)
        {
            state=true;
            if(onChange) onChange(state);  
            tLastChange=m;             
        }
    }
    
}

void Oscillator::begin(uint32_t _tOn, uint32_t _tOff, void (*_onChange)(bool state), bool _enabled)
{
    tOn=_tOn;
    tOff=_tOff;
    onChange=_onChange;
    enabled=_enabled;
}
