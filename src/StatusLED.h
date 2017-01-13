#ifndef StatusLED_h
#define StatusLED_h

#include <Arduino.h>
#include <Ticker.h>

class StatusLED {
public:
    StatusLED(int redPin, int greenPin, int bluePin);

    void setColor(int red, int green, int blue);

protected:
    int redPin, bluePin, greenPin;

    void setPWM(int red, int green, int blue);
};

#endif
