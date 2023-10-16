# Libraries/components used
# https://github.com/rpolitex/ArduinoNvs.git
# https://github.com/htmltiger/esp32-ds18b20.git


#pragma region include
#include "Arduino.h"
#include "Debounce.hpp"
#include "Toggle.hpp"
#include "Oscillator.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include <stdio.h>
#include "TempSens.h"
#include "nvsparameters.h"
#include "BinarySensor.h"
#include "Switch.h"
// #include "esp_timer.h"
#include "driver/uart.h"
#include "Proto485.h"
#include "OneWireESP32.h"
#include "ArduinoNvs.h"

#pragma endregion

#pragma region define
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
#define GPIO_SENSOR GPIO_NUM_23
#define GPIO_PUMP GPIO_NUM_18
#define GPIO_LED GPIO_NUM_22
#define GPIO_BUTTON GPIO_NUM_33
#define GPIO_VALVE GPIO_NUM_34
#define UARTTX GPIO_NUM_21
#define UARTRX GPIO_NUM_32
#define UARTRTS GPIO_NUM_19
#define UARTCTS UART_PIN_NO_CHANGE
#define WIFI_CONNECTED_BIT BIT0
#define OTA_BIT      BIT1
#define WIFI_CONNECTION_STATUS BIT2
#define MAXCMDLEN 200
#define FWNAME "fwcu4.bin"
#define SOLARPANELTEMPSENSOR 0
#define TANKTEMPSENSOR 1
#define FPTEMPSENSOR 2

#pragma endregion

extern "C" {
    void app_main(void);
}



#pragma region globals
uint8_t DT_ActPump=2; // if Tpanel > Ttank + DT_ActPump, then pump is acted
uint8_t Tread; // interval in seconds between temperature readings
OneWire32 ds(GPIO_SENSOR, 0, 1, 0);
//TempSens panelSensor(&a,"28b10056b5013caf"), tankSensor(&a,"282beb56b5013c7b"),fpSensor(&a,"282beb56b5013c7a");
static const char *TAG = "main";
Proto485 bus485(UART_NUM_2, UARTTX, UARTRX, UARTRTS, UARTCTS);
typedef unsigned char byte;
Debounce btnDebounce;
Toggle toggle1;
Oscillator solarCtrl,ledCtrl;
TempSens sensor[MaxDevs];
#pragma endregion



void onNewCommand(char *s)
{
    uint8_t err=0;
    const char *delim=" ";
    ESP_LOGI(TAG,"New command=%s",s);
    char *token = strtok(s, delim);
    if(!token) return;

    // wifi ssid
    if (strcmp(token,"wifissid")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("wifissid",token);
            return;
        }
    }

    // wifi password
    if (strcmp(token,"wifipassword")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("wifipassword",token);
            return;
        }
    }

    // ota url
    if (strcmp(token,"otaurl")==0 )
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("otaurl",token);
            esp_restart();
        }
    }

    // restart
    if (strcmp(token,"restart")==0)
    {
        esp_restart();
    }

    if (strcmp(token,"dtpump")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            uint8_t j=atoi(token);
                DT_ActPump=j;
                param.save("dtpump",DT_ActPump);
                return;
        }
    }


    if (strcmp(token,"tsendtemps")==0) // seconds between temp transmissions
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            uint16_t j=atoi(token);
            if(j)
            {
                param.save("tsendtemps",j);
                return;
            } 
        }
    }

    if (strcmp(token,"tread")==0) // in seconds
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            uint8_t j=atoi(token);
            if(j) {
                Tread=j;
                param.save("tread",Tread);
                return;
            }
        }
    }

    if (strcmp(token,"ton")==0) // in secs
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            uint8_t j=atoi(token);
            if(j) {param.save("ton",j); return;}
        }
    }

    if (strcmp(token,"toff")==0) // in secs
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            uint8_t j=atoi(token);
            if(j) {param.save("toff",j); return;}
        }
    }
    if (strcmp(token,"otacheck")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) {
            xEventGroupSetBits(event_group,OTA_BIT);
            return;
        }
    }

    if (strcmp(token,"psadd")==0) // panel sensor address
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("psadd",token);
            return;
        }
    }
    if (strcmp(token,"tsadd")==0) // tank sensor address
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("tsadd",token);
            return;
        }
    }

    if(err==1)
    {
        ESP_LOGE(TAG,"missing parameter\n");
        return;
    }
    if(err==2)
    {
        ESP_LOGE(TAG,"wrong value\n");
        return;
    }
    ESP_LOGE(TAG,"wrong command\n");
}

void ProcessStdin() {
    static char cmd[MAXCMDLEN];
    static uint8_t cmdlen=0;
    int c = fgetc(stdin);
    if(c!= EOF) 
    {
        ESP_LOGD(TAG,"%c",c);
        if(c=='\n') 
        {
            cmd[cmdlen]=0;
            onNewCommand(cmd);
            cmdlen=0;
        }
        else
        {
            cmd[cmdlen++]=c;
            if(cmdlen==MAXCMDLEN-1) 
            {
                cmdlen=0;
            }
        }
    }
}


void cmdSetTon(uint8_t* buf)
{
    uint8_t t=buf[1];
    solarPump.tOn=60000*t;
    param.save("ton",t);
}

void cmdSetToff(uint8_t* buf)
{
    uint8_t t=buf[1];
    solarPump.tOff=60000*t;
    param.save("toff",t);
}

void cmdSetDTACTPUMP(uint8_t* buf)
{
    DT_ActPump=buf[1];
    param.save("dtactpump",DT_ActPump);

}

void ProcessBusCommand(uint8_t cmd,uint8_t *buf,uint8_t len)
{
    ESP_LOGD(TAG,"cmd: %02x len: %d",cmd,len);
    ESP_LOG_BUFFER_HEX_LEVEL(TAG,buf,len,ESP_LOG_DEBUG);
    switch (cmd)
    {
        case CMD_STORE_CU_PARAM:
            switch (buf[0])
            {
                case PARAM_TON:
                    if (len==2) cmdSetTon(buf);
                    break;
                case PARAM_TOFF:
                    if (len==2) cmdSetToff(buf);
                    break;
                case PARAM_DTACTPUMP:
                    if (len==2) cmdSetDTACTPUMP(buf);
                    break;
                default:
                    //bus485.SendError("bad subparam");
                    break;

            };
            break;
        case CMDREQUESTSTATUS:
            bus485.SendStatus(solarPump.tOn/60000,solarPump.tOff/60000,DT_ActPump);
            break;
        case CMDRESTART:
            esp_restart();
            break;
        default:
            break;

    }

}

void onTempChange(float f)
{
    solarCtrl.enabled=(sensor[SOLARPANELTEMPSENSOR].getValue() > sensor[TANKTEMPSENSOR].getValue() + DT_ActPump);
}

void onSolarOutPin(bool b)
{
    digitalWrite(GPIO_J,(toggle1 || solarCtrl);
}

bool onIdle()
{
    static uint32_t lastTempCheck=0;
    uint32_t m=millis();
    if((millis() - lastTempCheck) > Tread*1000) {
        readTemperatures();
        lastTempCheck=m;
    } 
    solarCtrl.run();
    bus485.Rx();
    ProcessStdin();
    return false;
}


void searchSensors() {
    #define MaxDevs 3
    uint64_t addr[MaxDevs];
    uint8_t devices=ds.search(addr, MaxDevs);
	for (uint8_t i = 0; i < devices; i += 1) {
		Serial.printf("Sensor %d: 0x%llx,\n", i, addr[i]);
		//char buf[20]; snprintf( buf, 20, "0x%llx,", addr[i] ); Serial.println(buf);
	}
}

void readTemperatures()
{
    ds.request();
    vTaskDelay(750 / portTICK_PERIOD_MS);
    uint64_t a;
    float v;
    for(byte i = 0; i < MaxDevs; i++){
        a=sensor[i].addr;
        uint8_t err = ds.getTemp(&a, &v);
        if(err){
            const char *errt[] = {"", "CRC", "BAD","DC","DRV"};
            Serial.print(i); Serial.print(": "); Serial.println(errt[err]);
        }else{
            sensor[i].setValue(v);
            Serial.print(i); Serial.print(": "); Serial.println(v);
        }
    }
}


void app_main(void)
{
    NVS.begin();
    //esp_log_level_set("ds18b20", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_INFO);
    //esp_log_level_set("Switch", ESP_LOG_DEBUG);
    esp_log_level_set("main", ESP_LOG_INFO);
    searchSensors();
    param.Init();

    bus485.cbElaboraComando=&ProcessBusCommand;


    sensor[SOLARPANELTEMPSENSOR].begin(NVS.getInt("sptsadd"),onTempChange,[](float f) { bus485.SendPanelTemp(f); });
    sensor[TANKTEMPSENSOR].begin(NVS.getInt("tatsadd"),onTempChange,[](float f) { bus485.SendTankTemp(f); });


    solarCtrl.begin(NVS.getInt("ton"),NVS.getInt("toff"),onSolarOutPin,false);

    ledCtrl.begin(1000,1000,[](bool b) { digitalWrite(GPIO_LED,b); },true);

    btnDebounce.begin([]() {toggle1.toggle();});

    toggle1.begin(onSolarOutPin);
    attachInterrupt(digitalPinToInterrupt(GPIO_BUTTON), [](){btnDebounce.set(digitalRead(GPIO_BUTTON));}, CHANGE);
    
    
    param.load("dtactpump",&DT_ActPump);
    Tread=NVS.getInt("tread",10)
    esp_register_freertos_idle_hook(onIdle);

    ESP_LOGI(TAG,"Started. Tread=%u DT_ActPump=%u",Tread,DT_ActPump);

}