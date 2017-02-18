/*
  Sensors.h - Encapsulates MQTT / PubSubClient operations
  Created 26-JAN-2017 by Jon Brule
*/
#ifndef Sensors_h
#define Sensors_h

#include <Arduino.h>
#include <DHT.h>
#include <DHT_U.h>
#include "MQTT.h"

#define DHTTYPE DHT22

class Sensors
{
  public:
    Sensors(int doorPin, int dhtPin, int lightPin, int motionPin, int waterPin, MQTT* mqtt);
    void checkForAlerts();
    void handleDoorInterrupt();
    void handleMotionInterrupt();
    void measure();
    boolean readDoor(boolean read);
    float readHumidity(boolean read);
    int readLight(boolean read);
    boolean readMotion(boolean read);
    boolean readWater(boolean read);
    float readTemperature(boolean read);
    void setup();
  private:
    DHT_Unified* _dht;
    MQTT* _mqtt;
    int _doorPin;
    int _dhtPin;
    int _lightPin;
    int _motionPin;
    int _waterPin;
    volatile boolean _stateChange;
    volatile boolean _doorOpen;
    int _lightState;
    boolean _motionPresent;
    float _humidity;
    float _temperature;
    boolean _waterPresent;
    void _report(char* topic);
};

#endif
