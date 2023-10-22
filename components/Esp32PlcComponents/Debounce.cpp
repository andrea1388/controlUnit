#include "Debounce.hpp"
#include "Arduino.h"
Debounce::Debounce(const String &n)  : Base(n)
{
}
void Debounce::set(bool input)
{
    if(input != lastinput)
    {
        unsigned long m=millis();
        if((m-tLastChange) > tDebounce)
        {
            changed=true;
            tLastChange=m;
        }
        lastinput=input;
    }
}
void Debounce::begin(void (*_onClick)(), uint32_t _tDebounce)
{
    Base::begin(_onClick);
    tDebounce=_tDebounce;
}

