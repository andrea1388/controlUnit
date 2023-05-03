#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "TempSens.h"
#define PIN 23

extern "C" {
    void app_main(void);
}

void scanSensors(ds18b20 *a)
{
    DeviceAddressList address[3];
    int count=a->search_all(address,3);
}

void app_main(void)
{
    //esp_log_level_set("ds18b20", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("ds18b20", ESP_LOG_DEBUG);

    ds18b20 a((gpio_num_t)PIN);
    TempSens panelSensor(&a,"28b10056b5013caf"), tankSensor(&a,"282beb56b5013c7b");
    //panelSensor.setResolution(10);
    //tankSensor.setResolution(10);
    
    //scanSensors(&a);  // just to print sensor address

    while(true)
    {
        panelSensor.requestTemperatures(); // request for all sensors on the bus
        printf("paneltemp: %.1f mustsignal:%d\n", panelSensor.read(),panelSensor.mustSignal());
        printf("tanktemp: %.1f mustsignal:%d\n", tankSensor.read(),tankSensor.mustSignal());

        if(panelSensor.mustSignal())
        {
            panelSensor.Signaled();
            printf("panelSens signaled\n");
        }
    
        if(tankSensor.mustSignal())
        {
            tankSensor.Signaled();
            printf("tankSensor signaled\n");
        }
        vTaskDelay(1000);
    }
}
