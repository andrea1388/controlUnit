#include "Arduino.h"
class Debounce
{
private:
    /* data */
    bool lastinput=false;
    uint32_t tLastChange=0;
    uint32_t tDebounce=10;
    void (*onClick)()=0;
public:
    void set(bool input);
    void begin(void (*_onClick)(),uint32_t _tDebounce=10);
};


