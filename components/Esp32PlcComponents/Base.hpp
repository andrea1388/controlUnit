#ifndef __BASE_H__
#define __BASE_H__

#include "Arduino.h"

class Base
{
protected:
    String pName;
    bool changed=false;
    
public:
    Base(const String&);
    void begin(void (*onChange)());
    void (*onChange)()=0;
    void run();
};


#endif // __BASE_H__