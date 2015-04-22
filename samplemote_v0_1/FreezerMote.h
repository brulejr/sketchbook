/*
  Mote.h - Base library for sensor / control remote
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef FreezerMote_h
#define FreezerMote_h  

#include "Arduino.h"
#include "Mote.h"
#include <Message.h>

#include "BatterySensor.h"
#include "DoorSensor.h"
#include "LightSensor.h"
#include "TemperatureSensor.h"

#define DEBUG                1

#define BATTERY_PIN          0
#define DOOR_PIN             3
#define LIGHT_PIN            1
#define TEMP_INSIDE_PIN      2
#define TEMP_OUTSIDE_PIN     3

#define VOLTAGE              3.3

typedef struct SensorData {
  union {
    byte raw[MSG_DATA_LENGTH];
    struct {
      byte battery;      // battery voltage
      byte tempInside;   // temperature sensor: C * 10
      byte tempOutside;  // temperature sensor: C * 10
      byte light;        // light sensor: 0..255
      byte door;         // 0 or 1
    };
  };
};

class FreezerMote : public Mote {
  public:
    FreezerMote(const char* name, const char* version, bool init);
    ~FreezerMote();
    virtual void measure();
  protected:
    virtual void initConfig();
    virtual byte* sensorData();
  private:
    BatterySensor* _battery;
    DoorSensor* _door;
    LightSensor* _light;
    TemperatureSensor* _tempInside;
    TemperatureSensor* _tempOutside;
    SensorData _sensorData;
};

FreezerMote::FreezerMote(const char* name, const char* version, bool init = false) : Mote(name, version, init) {
  _battery = new BatterySensor(BATTERY_PIN);
  _door = new DoorSensor(DOOR_PIN);
  _light = new LightSensor(LIGHT_PIN);
  _tempInside = new TemperatureSensor(TEMP_INSIDE_PIN, VOLTAGE);
  _tempOutside = new TemperatureSensor(TEMP_OUTSIDE_PIN, VOLTAGE);
}

FreezerMote::~FreezerMote() {
  delete _battery, _door, _light, _tempInside, _tempOutside;
}

void FreezerMote::initConfig() {
  _rfNodeId = 9;
}

void FreezerMote::measure() {
  memset(&_sensorData, 0, sizeof(_sensorData));
  _sensorData.battery = _battery->read();
  _sensorData.door = _door->read();
  _sensorData.light = _light->read();
  _sensorData.tempInside = _tempInside->read();
  _sensorData.tempOutside = _tempOutside->read();
  #if DEBUG
    Serial.print("Sensors<battery = ");
    Serial.print(_sensorData.battery);
    Serial.print(", tempInside = ");    
    Serial.print(_sensorData.tempInside);
    Serial.print(", tempOutside = ");    
    Serial.print(_sensorData.tempOutside);
    Serial.print(", light = ");    
    Serial.print(_sensorData.light);
    Serial.print(", door = ");    
    Serial.print(_sensorData.door);
    Serial.println(">");
    Serial.flush();
  #endif
}

byte* FreezerMote::sensorData() {
  return (byte*) &_sensorData.raw;
}

#endif
