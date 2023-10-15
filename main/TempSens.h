#include "esp32ds18b20component.h"
class TempSens
{
    private:
        bool _mustsignal=false;
        float lastSignaledValue=0;
        uint32_t lastSignaledTime=0; // in milliseconds from boot
        ds18b20 *bus;
        DeviceAddress address;
    public:
        bool mustSignal();
        void Signaled();
        float read();
        float value=0;
        uint16_t minTimeBetweenSignal=10; // in seconds
        uint8_t minTempGapBetweenSignal=1;
        //TempSens(ds18b20*,DeviceAddress*);
        TempSens(ds18b20*,const char*);
        bool setResolution(uint8_t);
        void requestTemperatures();
        void SetAddress(const char*);
        void (*onChange)(float t);
        uint64_t addr;
        float value;
        void setValue(float v);

};