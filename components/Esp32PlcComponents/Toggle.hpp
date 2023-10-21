class Toggle
{
private:
    void (*onChange)(bool state);
public:
    bool state=false;
    void toggle();
    void begin(void (*_onChange)(bool state));
};




