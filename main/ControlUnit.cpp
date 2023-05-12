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

#define GPIO_SENSOR GPIO_NUM_23
#define GPIO_PUMP GPIO_NUM_18
#define GPIO_LED GPIO_NUM_4
#define GPIO_BUTTON GPIO_NUM_13
#define UARTTX GPIO_NUM_17
#define UARTRX GPIO_NUM_16
#define UARTRTS GPIO_NUM_18
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
Switch solarPump(GPIO_PUMP);
Switch statusLed(GPIO_LED);
BinarySensor pushButton(GPIO_BUTTON,GPIO_PULLDOWN_ONLY);
NvsParameters param;
static const char *TAG = "main";
Proto485 bus485(UART_NUM_2, UARTTX, UARTRX, UARTRTS, UARTCTS);
typedef unsigned char byte;
EventGroupHandle_t event_group;
WiFi wifi;
Otafw otafw;
char *otaurl; // otaurl https://otaserver:8070/SolarThermostat.bin
extern const uint8_t ca_crt_start[] asm("_binary_ca_crt_start");



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
    param.load("wifi_ssid",&ssid,"ssid");
    param.load("wifi_password",&password,"password");
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
    // mqtt uri
    if (strcmp(token,"mqtturi")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("mqtt_uri",token);
            return;
        }
    }

    // mqtt username
    if (strcmp(token,"mqttuname")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("mqtt_username",token);
            return;
        }
    }


    // wifi ssid
    if (strcmp(token,"wifissid")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("wifi_ssid",token);
            return;
        }
    }

    // wifi password
    if (strcmp(token,"wifipassword")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("wifi_password",token);
            return;
        }
    }

    // mqtt password
    if (strcmp(token,"mqttpwd")==0 )
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("mqtt_password",token);
            return;
        }
    }
    // mqtt_tptopic
    if (strcmp(token,"mqtt_tptopic")==0 )
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("mqtt_tptopic",token);
            return;
        }
    }
    if (strcmp(token,"mqtt_tttopic")==0 )
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("mqtt_tttopic",token);
            return;
        }
    }
    if (strcmp(token,"mqtt_cttopic")==0 )
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            param.save("mqtt_cttopic",token);
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
            int j=atoi(token);
            if(j<1 || j>256) err=2;
            if(err==0)
            {
                DT_ActPump=j;
                param.save("dt_actpump",DT_ActPump);
                return;
            } 
        }
    }


    if (strcmp(token,"tsendtemps")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            int j=atoi(token);
            if(j<1 || j>256) err=2;
            if(err==0)
            {
                param.save("Tsendtemps",j);
                return;
            } 
        }
    }

    if (strcmp(token,"tread")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            int j=atoi(token);
            if(j<1 || j>256) err=2;
            if(err==0) {
                Tread=j;
                param.save("Tread",Tread);
                return;
            }
        }
    }

    if (strcmp(token,"ton")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            int j=atoi(token);
            if(j<1 || j>256) err=2;
            if(err==0) {
                //Ton=j;
                param.save("Ton",j);
                return;
            }
        }
    }

    if (strcmp(token,"toff")==0)
    {
        token = strtok(NULL, delim);
        if(token==NULL) err=1;
        if(err==0) {
            int j=atoi(token);
            if(j<1 || j>256) err=2;
            if(err==0) {
                //Toff=j;
                param.save("Toff",j);
                return;
            }
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

    if(err==1)
    {
        printf("missing parameter\n");
        return;
    }
    if(err==2)
    {
        printf("wrong value\n");
        return;
    }
    printf("wrong command\n");
}

void ProcessStdin() {
    static char cmd[MAXCMDLEN];
    static uint8_t cmdlen=0;
    int c = fgetc(stdin);
    if(c!= EOF) 
    {
        printf("%c",c);
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
        default:
            break;

    }

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
    param.Init();
    event_group = xEventGroupCreate();
    bus485.cbElaboraComando=&ProcessBusCommand;

    panelSensor.setResolution(10);
    param.load("tsendtemps",&panelSensor.minTimeBetweenSignal);
    param.load("dttx",&panelSensor.minTempGapBetweenSignal);

    tankSensor.setResolution(10);
    tankSensor.minTimeBetweenSignal=panelSensor.minTimeBetweenSignal;
    tankSensor.minTempGapBetweenSignal=panelSensor.minTempGapBetweenSignal;

    statusLed.tOn=1000;
    statusLed.tOff=1000;

    solarPump.tOn=1000;
    solarPump.tOff=1000;
    param.load("ton",&solarPump.tOn);
    param.load("toff",&solarPump.tOff);
    param.load("dtactpump",&DT_ActPump);
    param.load("tread",&Tread);

    param.load("otaurl",&otaurl);
    if(otaurl) 
    {
        otafw.Init(otaurl,(const char*)ca_crt_start);
        xTaskCreate(&Ota, "ota_task", 8192, NULL, 5, NULL);
        ESP_LOGI(TAG,"Ota started");
    }

    //scanSensors(&a);  // just to print sensor address

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