#ifndef __DEBOUNCE_H__
#define __DEBOUNCE_H__

#include "Arduino.h"
#include "Base.hpp"
class Debounce : public Base
{
private:
    /* data */
    bool lastinput=false;
    uint32_t tLastChange=0;
    uint32_t tDebounce=10;
public:
    Debounce(const String&n);
    void set(bool input);
    void begin(void (*_onChange)(),uint32_t _tDebounce=10);

};



#endif // __DEBOUNCE_H__