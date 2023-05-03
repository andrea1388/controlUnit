#include "esp32ds18b20component.h"
class TempSens
{
    private:
        bool _mustsignal=false;
        float lastSignaledValue=0;
        uint8_t lastSignaledTime=0; // in milliseconds from boot
        ds18b20 *bus;
        DeviceAddress address;
    public:
        bool mustSignal();
        void Signaled();
        float read();
        float value;
        uint32_t minTimeBetweenSignal=1000;
        float minTempGapBetweenSignal=1.0;
        //TempSens(ds18b20*,DeviceAddress*);
        TempSens(ds18b20*,const char*);
        bool setResolution(uint8_t);
        void requestTemperatures();

};