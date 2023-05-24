#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <stdio.h>
#include "TempSens.h"
#include "nvsparameters.h"
#include "BinarySensor.h"
#include "Switch.h"
#include "esp_timer.h"
#include "driver/uart.h"
#include "millis.h"
#include "Proto485.h"
#include "wifi.h"
#include "otafw.h"

// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
#define GPIO_SENSOR GPIO_NUM_23
#define GPIO_PUMP GPIO_NUM_18
#define GPIO_LED GPIO_NUM_22
#define GPIO_BUTTON GPIO_NUM_33
#define UARTTX GPIO_NUM_21
#define UARTRX GPIO_NUM_32
#define UARTRTS GPIO_NUM_19
#define UARTCTS UART_PIN_NO_CHANGE
#define WIFI_CONNECTED_BIT BIT0
#define OTA_BIT      BIT1
#define MAXCMDLEN 200


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
Switch solarPump(GPIO_PUMP,GPIO_MODE_INPUT_OUTPUT,true);
Switch statusLed(GPIO_LED,GPIO_MODE_INPUT_OUTPUT,false);
BinarySensor pushButton(GPIO_BUTTON,GPIO_PULLDOWN_ONLY);
NvsParameters param;
static const char *TAG = "main";
Proto485 bus485(UART_NUM_2, UARTTX, UARTRX, UARTRTS, UARTCTS);
typedef unsigned char byte;
EventGroupHandle_t event_group;
WiFi wifi;
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_crt_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_crt_end");

Otafw otafw;
char *otaurl; // otaurl https://otasrv:8070/
#define FWNAME "fwcu3.bin"



void Ota(void *o) 
{
    ESP_LOGI(TAG, "Starting OTA task");
    for (;;) 
    {
        if(xEventGroupWaitBits(event_group,WIFI_CONNECTED_BIT | OTA_BIT, pdFALSE, pdTRUE, 1000/portTICK_PERIOD_MS) == (WIFI_CONNECTED_BIT | OTA_BIT)) 
        {
            ESP_LOGI(TAG, "Starting OTA update");
            otafw.Check();        
            xEventGroupClearBits(event_group, OTA_BIT);
        }
    }
}

void WiFiEvent(WiFi* wifi, uint8_t ev)
{
    switch(ev)
    {
        case WIFI_START: // start
            wifi->Connect();
            break;
        case WIFI_DISCONNECT: // disconnected
            xEventGroupClearBits(event_group, WIFI_CONNECTED_BIT);
            wifi->Connect();
            statusLed.tOn=1000;
            statusLed.tOff=1000;
            break;
        case WIFI_GOT_IP: // connected
            xEventGroupSetBits(event_group, WIFI_CONNECTED_BIT);
            ESP_LOGI(TAG,"GotIP");
            statusLed.tOn=100;
            statusLed.tOff=100;
            break;

    }
}

void WifiSetup()
{
    char *ssid=NULL;
    char *password=NULL;
    param.load("wifissid",&ssid,"ssid");
    param.load("wifipassword",&password,"password");
    wifi.onEvent=&WiFiEvent;
    if(ssid) wifi.Start(ssid,password);
    free(ssid);
    free(password);
}

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
    solarPump.tOn=1000*(buf[0]+buf[1]*256);
    param.save("ton",solarPump.tOn);
}

void cmdSetToff(uint8_t* buf)
{
    solarPump.tOff=1000*(buf[0]+buf[1]*256);
    param.save("toff",solarPump.tOff);
}

void cmdSetDTACTPUMP(uint8_t* buf)
{
    DT_ActPump=buf[0];
    param.save("dtactpump",DT_ActPump);
   
}

void ProcessBusCommand(uint8_t cmd,uint8_t *buf,uint8_t len)
{
    ESP_LOGD(TAG,"cmd: %02x len: %d",cmd,len);
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
                    if (len==1) cmdSetDTACTPUMP(buf);
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

void ReadTransmitTemp()
{
    panelSensor.requestTemperatures(); // request for all sensors on the bus
    ESP_LOGI(TAG,"paneltemp: %.1f mustsignal:%d\n", panelSensor.read(),panelSensor.mustSignal());
    ESP_LOGI(TAG,"tanktemp: %.1f mustsignal:%d\n", tankSensor.read(),tankSensor.mustSignal());
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


void ProcessThermostat() 
{
    pushButton.run();
    bool cond=(panelSensor.value > tankSensor.value + DT_ActPump) || pushButton.state;
    solarPump.run(cond);
    statusLed.run(true); // flash
    ESP_LOGD(TAG,"cond=%d butt=%d pump=%d",cond,pushButton.state,solarPump.State());
}


void app_main(void)
{
    //esp_log_level_set("ds18b20", ESP_LOG_DEBUG);
    esp_log_level_set("*", ESP_LOG_INFO);
    //esp_log_level_set("Proto485", ESP_LOG_DEBUG);
    esp_log_level_set("main", ESP_LOG_INFO);
    scanSensors(&a);  // just to print sensor address
    param.Init();
    event_group = xEventGroupCreate();
    bus485.cbElaboraComando=&ProcessBusCommand;

    param.load("tsendtemps",&panelSensor.minTimeBetweenSignal);
    param.load("dttx",&panelSensor.minTempGapBetweenSignal);
    char *add=NULL;
    param.load("psadd",&add);
    if(add) {panelSensor.SetAddress(add); free(add);}

    tankSensor.minTimeBetweenSignal=panelSensor.minTimeBetweenSignal;
    tankSensor.minTempGapBetweenSignal=panelSensor.minTempGapBetweenSignal;
    add=NULL;
    param.load("tsadd",&add);
    if(add) {tankSensor.SetAddress(add); free(add);}

    panelSensor.setResolution(10);
    tankSensor.setResolution(10);

    statusLed.tOn=1000;
    statusLed.tOff=1000;
    pushButton.toggle=true;

    uint8_t t=1;
    param.load("ton",&t);
    solarPump.tOn=t*1000*60;
    t=1;
    param.load("toff",&t);
    solarPump.tOff=t*1000*60;
    solarPump.inverted=true;
    
    param.load("dtpump",&DT_ActPump);
    param.load("tread",&Tread);

    param.load("otaurl",&otaurl);

    xEventGroupClearBits(event_group, WIFI_CONNECTED_BIT | OTA_BIT);

    WifiSetup();

    if(otaurl) 
    {
        char url[100];
        strcpy(url,otaurl);
        strcat(url,FWNAME);
        otafw.Init(url,(const char*)server_cert_pem_start);
        xTaskCreate(&Ota, "ota_task", 8192, NULL, 5, NULL);
        ESP_LOGI(TAG,"Ota started");
        xEventGroupSetBits(event_group,  OTA_BIT);
    }

    ESP_LOGI(TAG,"Starting. Tread=%u Ton=%lu Toff=%lu DT_ActPump=%u",Tread,solarPump.tOn,solarPump.tOff,DT_ActPump);


    uint32_t now,tLastRead=0;
    while(true)
    {
        now=millis();
        //ESP_LOGI(TAG,"millis: %ld tlr: %ld",now,tLastRead);
        if((now - tLastRead) > Tread*1000) {ReadTransmitTemp(); tLastRead=now;};
        ProcessThermostat();
        bus485.Rx();
        ProcessStdin();
        vTaskDelay(1);
    }
}