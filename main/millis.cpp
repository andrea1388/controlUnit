#include "esp_timer.h"
uint32_t millis() 
{
    return (esp_timer_get_time() >> 10) && 0xffffffff;
}