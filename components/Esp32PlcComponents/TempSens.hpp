#ifndef __TEMPSENS_H__
#define __TEMPSENS_H__

#include "Base.hpp"
class TempSens : public Base
{
    private:

    public:
        TempSens(const String&n);
        uint64_t addr=0;
        float value=0;
        void setValue(float v);
        void begin(uint64_t addr,  void (*onChange)());
};
#endif // __TEMPSENS_H__