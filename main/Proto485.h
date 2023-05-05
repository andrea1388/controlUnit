typedef unsigned char byte;

class Proto485 {
public:
    Proto485(Stream *stream, int pinTX, bool testreg);
    void ProcessaDatiSeriali(unsigned char c);
    void Tx(char cmd, byte len, const char* b);
    void (*cbElaboraComando)(byte comando,byte *bytesricevuti,byte len);
protected:
    char _txenablepin;
    Stream* _stream;
    bool _testreg;
};