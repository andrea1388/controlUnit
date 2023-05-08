#include "TempSens.h"
#include "millis.h"

bool TempSens::mustSignal()
{
    return _mustsignal;
}

bool TempSens::setResolution(uint8_t bit)
{
    return bus->setResolution((const DeviceAddress*)address,bit);
}

void TempSens::requestTemperatures()
{
    bus->requestTemperatures();
}

float TempSens::read()
{
/*     for (uint8_t i = 0; i < 8; i++){
		printf("%02x", address[i]);
	}
    printf(" read\n"); */
    value=bus->getTempC((const DeviceAddress*)address);
    if((lastSignaledTime-millis())>minTimeBetweenSignal) 
        if(abs(value-lastSignaledValue)>minTempGapBetweenSignal) _mustsignal=true;
    return value;
}

void TempSens::Signaled()
{

    lastSignaledValue=value;
    lastSignaledTime=millis();
    _mustsignal=false;
}

/* TempSens::TempSens(ds18b20* _bus,DeviceAddress* da)
{
    bus=_bus;
    for(uint8_t i=0;i<8;i++) _address[i]=*da[i];
} */

TempSens::TempSens(ds18b20* _bus,const char* hexstring)
{
    bus=_bus;
    //address=(DeviceAddress*)malloc(sizeof(DeviceAddress));
    ds18b20::HexToDeviceAddress(address,hexstring);
/*     for (uint8_t i = 0; i < 8; i++){
		printf("%02x", address[i]);
	}
    printf(" tempsens\n"); */
}