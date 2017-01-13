#include "StatusLED.h"

StatusLED::StatusLED(int redPin, int greenPin, int bluePin)
    : redPin(redPin), greenPin(greenPin), bluePin(bluePin) {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    setColor(0, 0, 0);
}

void StatusLED::setColor(int red, int green, int blue) {
    setPWM(red, green, blue);
}

void StatusLED::setPWM(int red, int green, int blue) {
    red = 1024 - (red*4);
    green = 1024 - (green*4);
    blue = 1024 - (blue*4);
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
}

