#include "esp32ds18b20component.h"
class TempSens
{
    private:
        uint32_t lastSignaledTime=0; // in milliseconds from boot
        uint16_t minTimeBetweenSignal=10; // in seconds
        uint8_t minTempGapBetweenSignal=1;
        void (*onChange)(float t)=0;
        void (*onSignal)(float t)=0;
        uint64_t addr=0;

    public:
        float value=0;
        void setValue(float v);
        void begin(uint64_t addr,  void (*onChange)(float t), void (*onSignal)(float t), uint16_t minTimeBetweenSignal=10, uint8_t minTempGapBetweenSignal=1);

};