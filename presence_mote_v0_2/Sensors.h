/*
  Sensors.h - Encapsulates MQTT / PubSubClient operations
  Created 26-JAN-2017 by Jon Brule
*/
#ifndef Sensors_h
#define Sensors_h

#include "Arduino.h"

class Sensors
{
  public:
    Sensors(int doorPin, int lightPin, int motionPin);
    int readDoor(bool read);
    int readLight(bool read);
    int readMotion(bool read);
  private:
    int _doorPin;
    int _lightPin;
    int _motionPin;
    int _doorState;
    int _lightState;
    int _motionState;
};

#endif
