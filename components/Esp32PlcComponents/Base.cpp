#include "Base.hpp"
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
char *plctag="plc";


Base::Base(const String &name) 
{
    pName=name;
}

void Base::begin(void (*_onChange)())
{
    onChange=_onChange;
    ESP_LOGI(plctag,"begin:%s",pName.c_str());
}

void Base::run()
{
    ESP_LOGD(plctag,"run:%s",pName.c_str());
    if(changed)
    {
        ESP_LOGD(plctag,"change:%s",pName.c_str());
        if(onChange) (*onChange)();
        changed=false;    
    }
}