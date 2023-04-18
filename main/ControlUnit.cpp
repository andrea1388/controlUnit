#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "TempSens.h"
#define PIN 2

extern "C" {
    void app_main(void);
}
void app_main(void)
{
    DeviceAddress address[2];
    ds18b20 a(2);
    TempSens* s[2];
    int count=a.search_all(address);
    for(int i=0;i<count;i++)
    {
        s[i]=new TempSens(&a,address[i]);
    }
    while(true)
    {
        a.requestTemperatures();
        for(int i=0;i<count;i++)
        {
            printf("s: %d v:%f",i,s[i]->read());
        }
        vTaskDelay(1000);
    }
}
