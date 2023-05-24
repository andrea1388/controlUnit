#include "driver/gpio.h"
#include "BinarySensor.h"
//#include "string.h"
#include "esp_timer.h"
#include "esp_log.h"
static const char *TAG = "BinarySensor";


BinarySensor::BinarySensor(gpio_num_t _pin,gpio_pull_mode_t mode)
{
    pin=_pin;
    debounceTime = 50;
    ESP_ERROR_CHECK(gpio_reset_pin(pin));
    ESP_ERROR_CHECK(gpio_set_direction(pin, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(pin,mode));
    tLastReading=0;
    toggle=false;
    state=false;
/*     gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
 */
}

void BinarySensor::run()
{
    if (debouceTimeElapsed())
        processInput();
}

void BinarySensor::processInput()
{
    static bool lastinput=false;
    bool input=(gpio_get_level(pin)==1);
    
    ESP_LOGD(TAG,"inp= %d, state=%d",input, state);

    if(toggle)
    {
        if(input && !lastinput) state=!state;
    }
    else
        state = input;
    lastinput=input;
}

bool BinarySensor::debouceTimeElapsed()
{
    int64_t now = esp_timer_get_time();
    if ((now - tLastReading) > debounceTime*1000) {
        tLastReading=now;
        return true;
    }
    return false;
}
