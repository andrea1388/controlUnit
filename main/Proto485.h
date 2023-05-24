#include "driver/uart.h"
typedef unsigned char byte;

#define CMDSENDPANELTEMP 'i'
#define CMDSENDTANKTEMP 'j'
#define CMD_STORE_CU_PARAM 'k'
#define PARAM_TON 'A'
#define PARAM_TOFF 'B'
#define PARAM_DTACTPUMP 'C'
#define CMDREQUESTSTATUS 'l'
#define CMDSENDSTATUS 'm'
#define CMDRESTART 'n'

class Proto485 {
public:
    Proto485(uart_port_t uart_num, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num);
    void ProcessaDatiSeriali(unsigned char c);
    void Tx(char cmd, byte len, const char* b);
    void Rx();
    void (*cbElaboraComando)(byte comando,byte *bytesricevuti,byte len);
    void SendPanelTemp(float);
    void SendTankTemp(float);
    void SendStatus(uint8_t ton, uint8_t toff, uint8_t DT_ActPump);
protected:
    uart_port_t uart_num;

};