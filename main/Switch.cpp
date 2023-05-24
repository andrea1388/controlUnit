#include "driver/gpio.h"
#include "Switch.h"
#include "esp_timer.h"
//#include "string.h"
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
static const char *TAG = "Switch";

Switch::Switch(gpio_num_t _pin,gpio_mode_t mode,bool _inverted) {
    pin=_pin;
    tOn=tOff=0;
    on=off=false;
    toggleMode=false;
    ESP_ERROR_CHECK(gpio_reset_pin(pin));
    ESP_ERROR_CHECK(gpio_set_direction(pin, mode));
    inverted=_inverted;
    changeState(false);
}

void Switch::changeState(bool s) 
{
    if(inverted) s=!s;
    if(s) gpio_set_level(pin,1); else gpio_set_level(pin,0); 
    tLastChange=esp_timer_get_time();
}

void Switch::run(bool inp) {
    bool out=(gpio_get_level(pin)==1);
    if(inverted) out=!out;
    int64_t now=esp_timer_get_time();
    

    uint8_t modo;
    if(toggleMode) modo=1; else if(!tOn) modo=2; else modo=3;
    ESP_LOGD(TAG,"modo: %u input: %d previnp: %d out: %d time: %lld lastchange: %lld Ton:%ld Toff:%ld\n", modo, inp,previnp,out,now/1000,tLastChange/1000,tOn,tOff);


    if(!out) {
        if(on) { on=false;changeState(true);return;}
        switch (modo)
        {
        case 1:
        case 2:
            if(inp && !previnp) {changeState(true);break;}
            break;
        case 3:
            if((inp && !previnp) || (inp && (now-tLastChange)>tOff*1000)) { changeState(true);break;}
            break;
        default:
            assert(0);
            break;
        }
    } 
    else {
        if(off) { off=false;changeState(false);return;}
        switch (modo)
        {
        case 1:
            if(inp && !previnp) { changeState(false);break;}
            if(tOn && (now-tLastChange)>tOn*1000) { changeState(false);break;}
            break;
        case 2:
        case 3:
            if(!inp && previnp) { changeState(false);break;}
            if(tOn && (now-tLastChange)>tOn*1000) { changeState(false);break;}
            break;
        
        default:
            assert(0);
            break;
        }
    }
    previnp=inp;
}

bool Switch::State() 
{
    bool out=(gpio_get_level(pin)==1);
    if(inverted) return !out;
    return out;
}

