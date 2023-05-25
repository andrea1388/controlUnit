#include "esp_log.h"
#include "Proto485.h"
#include "millis.h"
#include "soc/uart_reg.h"
#include "soc/soc.h"
//#define DEBUG485
#define LMAXPKT 20
#define A 0
#define L 1
#define D 2
#define C 3
#define S 4
static const char *TAG = "Proto485";

Proto485::Proto485(uart_port_t _uart_num, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num) 
{
  uart_num=_uart_num;
  uart_config_t uart_config = {
  .baud_rate = 9600,
  .data_bits = UART_DATA_8_BITS,
  .parity = UART_PARITY_DISABLE,
  .stop_bits = UART_STOP_BITS_1,
  .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  .rx_flow_ctrl_thresh = 122,
  .source_clk = UART_SCLK_DEFAULT
  };
  REG_SET_BIT(UART_CONF1_REG(uart_num),UART_RS485TX_RX_EN);


  // Configure UART parameters
  ESP_ERROR_CHECK(uart_driver_install(uart_num,127*2 , 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(uart_num, tx_io_num, rx_io_num, rts_io_num, cts_io_num));
  ESP_ERROR_CHECK(uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX));
}

void Proto485::ProcessaDatiSeriali(unsigned char c) {
  static byte numerobytesricevuti=0,bytesricevuti[LMAXPKT],prossimodato=A,lunghezza,comando,sum;
  static unsigned long tultimodatoricevuto;
  ESP_LOGD(TAG, "c=%02x prox=%02x",c,prossimodato);
  if(millis()-tultimodatoricevuto > 300) {prossimodato=A; };
  tultimodatoricevuto=millis();
  if(prossimodato==A && c=='A') {prossimodato=C; numerobytesricevuti=0; return;}
  if(prossimodato==C) {comando=c; prossimodato=L; sum=c; return;}
  if(prossimodato==L) {
    sum+=c;
    lunghezza=c;
    prossimodato=D; 
    if(lunghezza>LMAXPKT) prossimodato=A;
    if(lunghezza==0) {
      prossimodato=S;
    }
    ESP_LOGD(TAG, "next D l=%d",lunghezza);
    return;
  }
  if(prossimodato==D) {
    sum+=c;
    bytesricevuti[numerobytesricevuti++]=c;
    if(numerobytesricevuti==lunghezza) prossimodato=S;
    return;
  }
  if(prossimodato==S) {
    if(c==sum) {
      if (cbElaboraComando) this->cbElaboraComando(comando,bytesricevuti,lunghezza);
      prossimodato=A;
    } else {
      ESP_LOGE(TAG, "wrong checksum");
    }
  }
}

void Proto485::Tx(char cmd, byte len, const char* b) 
{
    byte sum=(byte)cmd+len;
    uart_write_bytes(uart_num, "A", 1);
    uart_write_bytes(uart_num, &cmd, 1);
    uart_write_bytes(uart_num, &len, 1);
    if(len)
    {
      uart_write_bytes(uart_num, b, len);
      for (byte f=0;f<len;f++) {
        sum+=b[f];
      }
    }
    uart_write_bytes(uart_num, &sum, 1);
}

void Proto485::Rx()
{
  unsigned char c;
  uint8_t len = uart_read_bytes(uart_num, &c, 1, 1);
  if(len >0) ProcessaDatiSeriali(c);
}

void Proto485::SendPanelTemp(float val)
{
  union {
    float value;
    byte data[4];
  } u;
  u.value=val;
  Tx(CMDSENDPANELTEMP, 4, (const char *)u.data); 
}

void Proto485::SendTankTemp(float val)
{
  union {
    float value;
    byte data[4];
  } u;
  u.value=val;
  Tx(CMDSENDTANKTEMP, 4, (const char *)u.data); 
}

void Proto485::SendStatus(uint8_t ton, uint8_t toff, uint8_t DT_ActPump)
{
  uint8_t buf[3];
  buf[0]=ton;
  buf[1]=toff;
  buf[2]=DT_ActPump;
  Tx(CMDSENDSTATUS, 3, (const char *)buf); 
}