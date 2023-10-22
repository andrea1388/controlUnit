#ifndef __OSCILLATOR_H__
#define __OSCILLATOR_H__

#include "Base.hpp"
class Oscillator : public Base
{
private:
    /* data */
    unsigned long tLastChange=0;

public:
    Oscillator(const String&n);
    uint32_t tOn=10,tOff=10;
    bool state=false;
    void run(); // must be called frequently
    bool enabled=false;
    void begin(uint32_t tOn, uint32_t tOff, void (*onChange)(), bool enabled=false);
};

#endif // __OSCILLATOR_H__