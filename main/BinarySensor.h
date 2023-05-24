# pragma once
#include "driver/gpio.h"
class BinarySensor 
{
    public:
        BinarySensor(gpio_num_t pin,gpio_pull_mode_t);
        uint8_t debounceTime;
        void run();
        bool state;
        gpio_num_t pin;
        bool toggle;

    private:
        int64_t tLastReading;
        void processInput();
        bool debouceTimeElapsed();
};
