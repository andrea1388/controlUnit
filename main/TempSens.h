#include "esp32ds18b20component.h"
class TempSens
{
    private:
        bool _mustsignal;
        float lastSignaledValue;
        uint8_t lastSignaledTime; // in milliseconds from boot
        ds18b20 *bus;
        DeviceAddress address;
    public:
        bool mustSignal();
        void Signaled();
        float read();
        float value;
        uint32_t minTimeBetweenSignal;
        float minTempGapBetweenSignal;
        TempSens(ds18b20*,DeviceAddress*);

};