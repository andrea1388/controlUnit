#include "driver/uart.h"
typedef unsigned char byte;

#define CMDSENDPANELTEMP 'i'
#define CMDSENDTANKTEMP 'j'

class Proto485 {
public:
    Proto485(uart_port_t uart_num, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num);
    void ProcessaDatiSeriali(unsigned char c);
    void Tx(char cmd, byte len, const char* b);
    void Rx();
    void (*cbElaboraComando)(byte comando,byte *bytesricevuti,byte len);
    void SendPanelTemp(float);
    void SendTankTemp(float);
protected:
    uart_port_t uart_num;

};