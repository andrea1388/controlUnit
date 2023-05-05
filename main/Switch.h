# pragma once
#include "driver/gpio.h"
#include "BinarySensor.h"


class Switch {
    public:
        bool toggleMode,on,off;
        uint32_t tOn; /*!< On time interval expressed in milliseconds */
        uint32_t tOff; /*!< Off time interval expressed in milliseconds */
        gpio_num_t pin;
        void run(bool);
        Switch(gpio_num_t pin);
    private:
        void changeState(bool);
        bool lastReading;
        int64_t tLastChange;
        bool cycle;
        bool previnp=false;
};
