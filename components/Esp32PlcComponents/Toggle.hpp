#ifndef __TOGGLE_H__
#define __TOGGLE_H__

#include "Base.hpp"
class Toggle : public Base
{
private:

public:
    Toggle(const String&n);
    bool state=false;
    void toggle();
    void begin(void (*_onChange)());

};





#endif // __TOGGLE_H__