#include "TempSens.h"

bool TempSens::mustSignal()
{
    return _mustsignal;
}

float TempSens::read()
{
    value=bus->getTempC(&address);
    if(abs(value-lastSignaledValue))>minTempGapBetweenSigna) _mustsignal=true;
    else
    if((lastSignaledTime-now)>minTimeBetweenSignal) _mustsignal=true;
    return value;
}

void TempSens::Signaled()
{

    lastSignaledValue=value;
    lastSignaledTime=now;
    _mustsignal=false;
}

TempSens::TempSens(ds18b20* _bus,DeviceAddress* da)
{
    bus=_bus;
    for(uint8_t i=0;i<8;i++) address[i]=*da[i];
}