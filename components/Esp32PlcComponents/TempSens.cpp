#include "Arduino.h"
#include "TempSens.h"

void TempSens::setValue(float v)
{
    if(v!=value) 
    {
        bool s=false;
        if( ((lastSignaledTime-millis())*1000>minTimeBetweenSignal) && (abs(v-value)>minTempGapBetweenSignal)) s=true;
        value=v;
        
        if(onChange) onChange(value);
        if(s) if(onSignal) onSignal(value);

    }

}

void TempSens::begin(uint64_t _addr, void (*_onChange)(float t), void (*_onSignal)(float t), uint16_t _minTimeBetweenSignal, uint8_t _minTempGapBetweenSignal)
{
    addr=_addr;
    minTimeBetweenSignal=_minTempGapBetweenSignal;
    minTempGapBetweenSignal=_minTempGapBetweenSignal;
    onChange=_onChange;
    onSignal=_onSignal;


}
