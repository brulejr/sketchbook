/*
  Sensors.h - Encapsulates MQTT / PubSubClient operations
  Created 18-FEB-2017 by Jon Brule
*/
#ifndef Sensors_h
#define Sensors_h

#include <Arduino.h>
#include <DHT.h>
#include <DHT_U.h>
#include "MQTT.h"

#define DHTTYPE DHT22
#define BAD_READING -999
#define DEFAULT_INTERRUPT_TIMER 200

class Sensors
{
  public:
    Sensors(int doorPin, 
            int dhtPin,
            int lightPin,
            int motionPin,
            int waterPin,
            MQTT* mqtt,
            int interruptTimer = DEFAULT_INTERRUPT_TIMER);
    void checkForAlerts();
    void handleDoorInterrupt();
    void handleMotionInterrupt();
    void handleWaterInterrupt();
    void measure();
    void setup();
  private:
    DHT_Unified* _dht;
    MQTT* _mqtt;
    int _doorPin;
    int _dhtPin;
    int _lightPin;
    int _motionPin;
    int _waterPin;
    int _interruptTimer;
    volatile boolean _stateChange;
    volatile boolean _doorOpen;
    volatile boolean _motionPresent;
    volatile boolean _waterPresent;
    int _lightState;
    float _humidity;
    float _temperature;
    boolean _readDoor();
    float _readHumidity();
    int _readLight();
    boolean _readMotion();
    boolean _readWater();
    float _readTemperature();
    void _report(char* topic);
};

#endif
