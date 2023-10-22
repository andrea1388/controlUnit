#include "Toggle.hpp"
Toggle::Toggle(const String &n) : Base(n)
{
}
void Toggle::toggle()
{
    state=!state;
    changed=true;   
     
}
void Toggle::begin(void (*_onChange)())
{
    onChange=_onChange;
}
