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
    Sensors(int doorPin, int lightPin, int motionPin, int waterPin, MQTT* mqtt);
    void check();
    void interrupt();
    boolean readDoor(boolean read);
    int readLight(boolean read);
    boolean readMotion(boolean read);
    boolean readWater(boolean read);
  private:
    MQTT* _mqtt;
    int _doorPin;
    int _lightPin;
    int _motionPin;
    int _waterPin;
    volatile boolean _stateChange;
    volatile boolean _doorOpen;
    int _lightState;
    boolean _motionPresent;
    boolean _waterPresent;
    void _report(char* topic);
};

#endif
