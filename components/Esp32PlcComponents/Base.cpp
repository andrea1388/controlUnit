#include "Base.hpp"
#include "esp_log.h"


Base::Base(const String &name) 
{
    pName=name;
}

void Base::begin(void (*_onChange)())
{
    onChange=_onChange;
    ESP_LOGI(tag,"begin:%s",pName.c_str());
}

void Base::run()
{
    ESP_LOGD(tag,"run:%s",pName.c_str());
    if(changed)
    {
        ESP_LOGI(tag,"change:%s",pName.c_str());
        if(onChange) (*onChange)();
        changed=false;    
    }
}