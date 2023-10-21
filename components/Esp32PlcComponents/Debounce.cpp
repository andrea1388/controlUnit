#include "Debounce.hpp"
#include "Arduino.h"
void Debounce::set(bool input)
{
    if(input)
    {
        if(!lastinput)
        {
            uint32_t m=millis();
            if((m-tLastChange) > tDebounce)
                if (onClick) onClick();
            tLastChange=m;
        }
    }
    lastinput=input;
}
void Debounce::begin(void (*_onClick)(), uint32_t _tDebounce)
{
    onClick=_onClick;
    tDebounce=_tDebounce;
}
