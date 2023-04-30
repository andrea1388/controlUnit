#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "TempSens.h"
#define PIN 23

extern "C" {
    void app_main(void);
}
void app_main(void)
{
    esp_log_level_set("ds18b20", ESP_LOG_DEBUG);
    DeviceAddress address[2];
    ds18b20 a((gpio_num_t)PIN);
    TempSens* s[2];
    int count=a.search_all(address);
    printf("sens found: %d\n",count);
    if(count>2) return;
    for(int i=0;i<count;i++)
    {
        s[i]=new TempSens(&a,&address[i]);
        s[i]->minTempGapBetweenSignal=1.7;
        s[i]->minTimeBetweenSignal=2000;
    }
    while(true)
    {
        a.requestTemperatures();
        for(int i=0;i<count;i++)
        {
            printf("sens: %d value:%f mustsignal:%d\n",i,s[i]->read(),s[i]->mustSignal());
            if(s[i]->mustSignal())
            {
                s[i]->Signaled();
                printf("signaled\n");
            }
        }
        vTaskDelay(1000);
    }
}
