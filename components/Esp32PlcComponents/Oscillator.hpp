class Oscillator
{
private:
    /* data */
    uint32_t tLastChange=0;
    void (*onChange)(bool state);
public:
    uint32_t tOn=10,tOff=10;
    bool state=false;
    void run(); // must be called frequently
    bool enabled=false;
    void begin(uint32_t tOn, uint32_t tOff, void (*onChange)(bool state), bool enabled=false);
};
