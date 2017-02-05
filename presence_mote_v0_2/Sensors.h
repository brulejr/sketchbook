/*
  Sensors.h - Encapsulates MQTT / PubSubClient operations
  Created 26-JAN-2017 by Jon Brule
*/
#ifndef Sensors_h
#define Sensors_h

#include "Arduino.h"
#include "MQTT.h"

class Sensors
{
  public:
    Sensors(int doorPin, int lightPin, int motionPin, MQTT* mqtt);
    void check();
    void interrupt();
    int readDoor(bool read);
    int readLight(bool read);
    int readMotion(bool read);
  private:
    MQTT* _mqtt;
    int _doorPin;
    int _lightPin;
    int _motionPin;
    volatile boolean _stateChange;
    volatile int _doorState;
    int _lightState;
    int _motionState;
    void _report(char* topic);
};

#endif
