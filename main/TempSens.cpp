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
