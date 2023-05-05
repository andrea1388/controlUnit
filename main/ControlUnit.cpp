#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "TempSens.h"
#include "nvsparameters.h"
#include "BinarySensor.h"
#include "Switch.h"
#include "esp_timer.h"
#include "Proto485.h"


#define GPIO_SENSOR GPIO_NUM_23
#define GPIO_PUMP GPIO_NUM_18
#define GPIO_LED GPIO_NUM_4
#define GPIO_BUTTON GPIO_NUM_13

extern "C" {
    void app_main(void);
}

void scanSensors(ds18b20 *a)
{
    DeviceAddressList address[3];
    a->search_all(address,3);
}

uint8_t DT_ActPump=2; // if Tpanel > Ttank + DT_ActPump, then pump is acted
uint8_t Tread=1; // interval in seconds between temperature readings

ds18b20 a((gpio_num_t)GPIO_SENSOR);
TempSens panelSensor(&a,"28b10056b5013caf"), tankSensor(&a,"282beb56b5013c7b");
Switch solarPump(GPIO_PUMP);
Switch statusLed(GPIO_LED);
BinarySensor pushButton(GPIO_BUTTON,GPIO_PULLDOWN_ONLY);
NvsParameters param;
static const char *TAG = "main";
Proto485 bus485;


void ProcessBusCommand(byte comando,byte *bytesricevuti,byte len)
{

}

void ReadTransmitTemp()
{
    panelSensor.requestTemperatures(); // request for all sensors on the bus
    printf("paneltemp: %.1f mustsignal:%d\n", panelSensor.read(),panelSensor.mustSignal());
    printf("tanktemp: %.1f mustsignal:%d\n", tankSensor.read(),tankSensor.mustSignal());
    if(panelSensor.mustSignal())
    {
        bus485.SendPanelTemp(panelSensor.value);
        panelSensor.Signaled();
    }

    if(tankSensor.mustSignal())
    {
        bus485.SendTankTemp(tankSensor.value);
        tankSensor.Signaled();
    }

}

void ProcessSerialData()
{
    const uart_port_t uart_num = UART_NUM_2;
    uint8_t data[128];
    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, (size_t*)&length));
    length = uart_read_bytes(uart_num, data, length, 100);
    for(uint8_t i=0;i<length;i++) bus485.ProcessaDatiSeriali(data[i]);
}
void ProcessThermostat() 
{
    pushButton.run();
    bool cond=(panelSensor.value > tankSensor.value + DT_ActPump) || pushButton.state;
    solarPump.run(cond);
    statusLed.run(true); // flash
    ESP_LOGI(TAG,"cond=%d butt=%d",cond,pushButton.state);
}
uint32_t millis() 
{
    return (esp_timer_get_time() >> 10) && 0xffffffff;
}

void app_main(void)
{
    //esp_log_level_set("ds18b20", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("ds18b20", ESP_LOG_DEBUG);

    bus485.cbElaboraComando=&ProcessBusCommand;

    panelSensor.setResolution(10);
    param.load("Tsendtemps",&panelSensor.minTimeBetweenSignal);
    param.load("DT_TxMqtt",&panelSensor.minTempGapBetweenSignal);

    tankSensor.setResolution(10);
    tankSensor.minTimeBetweenSignal=panelSensor.minTimeBetweenSignal;
    tankSensor.minTempGapBetweenSignal=panelSensor.minTempGapBetweenSignal;

    statusLed.tOn=4000;
    statusLed.tOff=4000;

    solarPump.tOn=1*60*1000;
    solarPump.tOff=2*60*1000;
    param.load("Ton",&solarPump.tOn);
    param.load("Toff",&solarPump.tOff);

    scanSensors(&a);  // just to print sensor address

    uint32_t now,tLastRead=0;
    while(true)
    {
        now=millis();
        if((now - tLastRead) > Tread*1000) {ReadTransmitTemp(); tLastRead=now;};
        ProcessThermostat();
        ProcessSerialData();
        vTaskDelay(1);
    }
}