#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "TempSens.h"
#include "nvsparameters.h"
#include "BinarySensor.h"
#include "Switch.h"
#include "esp_timer.h"
#include "driver/uart.h"
#include "millis.h"
#include "Proto485.h"


#define GPIO_SENSOR GPIO_NUM_23
#define GPIO_PUMP GPIO_NUM_18
#define GPIO_LED GPIO_NUM_4
#define GPIO_BUTTON GPIO_NUM_13
#define UARTTX GPIO_NUM_17
#define UARTRX GPIO_NUM_16
#define UARTRTS GPIO_NUM_18
#define UARTCTS UART_PIN_NO_CHANGE

extern "C" {
    void app_main(void);
}

void scanSensors(ds18b20 *a)
{
    DeviceAddressList address[3];
    a->search_all(address,3);
}

uint8_t DT_ActPump=2; // if Tpanel > Ttank + DT_ActPump, then pump is acted
uint8_t Tread=30; // interval in seconds between temperature readings

ds18b20 a((gpio_num_t)GPIO_SENSOR);
TempSens panelSensor(&a,"28b10056b5013caf"), tankSensor(&a,"282beb56b5013c7b");
Switch solarPump(GPIO_PUMP);
Switch statusLed(GPIO_LED);
BinarySensor pushButton(GPIO_BUTTON,GPIO_PULLDOWN_ONLY);
NvsParameters param;
static const char *TAG = "main";
Proto485 bus485(UART_NUM_2, UARTTX, UARTRX, UARTRTS, UARTCTS);
typedef unsigned char byte;


void ProcessBusCommand(byte comando,byte *bytesricevuti,byte len)
{
    ESP_LOGD(TAG,"cmd: %02x len: %d",comando,len);

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
        //bus485.SendTankTemp(tankSensor.value);
        tankSensor.Signaled();
    }

}


void ProcessThermostat() 
{
    pushButton.run();
    bool cond=(panelSensor.value > tankSensor.value + DT_ActPump) || pushButton.state;
    solarPump.run(cond);
    statusLed.run(true); // flash
    //ESP_LOGI(TAG,"cond=%d butt=%d",cond,pushButton.state);
}


void app_main(void)
{
    //esp_log_level_set("ds18b20", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("ds18b20", ESP_LOG_INFO);
    esp_log_level_set("main", ESP_LOG_DEBUG);
    
    bus485.cbElaboraComando=&ProcessBusCommand;

    panelSensor.setResolution(10);
    param.load("Tsendtemps",&panelSensor.minTimeBetweenSignal);
    param.load("DT_TxMqtt",&panelSensor.minTempGapBetweenSignal);

    tankSensor.setResolution(10);
    tankSensor.minTimeBetweenSignal=panelSensor.minTimeBetweenSignal;
    tankSensor.minTempGapBetweenSignal=panelSensor.minTempGapBetweenSignal;

    statusLed.tOn=100;
    statusLed.tOff=100;

    solarPump.tOn=1000;
    solarPump.tOff=1000;
    param.load("Ton",&solarPump.tOn);
    param.load("Toff",&solarPump.tOff);

    //scanSensors(&a);  // just to print sensor address

    uint32_t now,tLastRead=0;
    while(true)
    {
        now=millis();
        //ESP_LOGI(TAG,"millis: %ld tlr: %ld",now,tLastRead);
        if((now - tLastRead) > Tread*1000) {ReadTransmitTemp(); tLastRead=now;};
        ProcessThermostat();
        bus485.Rx();
        vTaskDelay(1);
    }
}