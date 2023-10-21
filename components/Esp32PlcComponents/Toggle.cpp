#include "Toggle.hpp" 
void Toggle::toggle()
{
    state=!state;
    if(onChange) (*onChange)(state);
}
void Toggle::begin(void (*_onChange)(bool state))
{
    onChange=_onChange;
}
