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

#define BATTERY_PIN          A0
#define DOOR_PIN             3
#define LIGHT_PIN            A1
#define TEMP_INSIDE_PIN      A2
#define TEMP_OUTSIDE_PIN     A3

#define VOLTAGE              3.3
#define WARNING_PERIOD_IN_S  10

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
    virtual unsigned int calculateLedDelay();
    virtual byte calculateMessageLevel();
    virtual byte calculateSleepMultiplier();
    virtual void initConfig();
    virtual byte* sensorData();
    virtual void setupPorts();
  private:
    bool isAlert();
    bool isNormal();
    BatterySensor* _battery;
    DoorSensor* _door;
    LightSensor* _light;
    TemperatureSensor* _tempInside;
    TemperatureSensor* _tempOutside;
    SensorData _sensorData;    
};

//-----------------------------------------------------------------------------
// API Methods
//-----------------------------------------------------------------------------
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


//-----------------------------------------------------------------------------
// Support Methods
//-----------------------------------------------------------------------------
unsigned int FreezerMote::calculateLedDelay() {
  if (isNormal()) {
    return isAlert() ? 500: 250;
  } else {
    return 100;
  }
}

byte FreezerMote::calculateMessageLevel() {
  if (isNormal()) {
    return isAlert() ? MSG_ALERT: MSG_WARNING;
  } else {
    return MSG_READING;
  }
}

byte FreezerMote::calculateSleepMultiplier() {
  if (isNormal()) {
    return isAlert() ? _alertMultiplier: _alertMultiplier * 2;
  } else {
    return _loopMultiplier;
  }

}

void FreezerMote::initConfig() {
  _rfNodeId = 9;
}

bool FreezerMote::isAlert() {
  return millis() > _lastIntrTime + (WARNING_PERIOD_IN_S * 1000);
}

bool FreezerMote::isNormal() {
  return _sensorData.door;
}

void FreezerMote::measure() {
  memset(&_sensorData, 0, sizeof(_sensorData));
  _sensorData.battery = _battery->read();
  _sensorData.door = _door->read();
  _sensorData.light = _light->read();
  _sensorData.tempInside = _tempInside->read();
  _sensorData.tempOutside = _tempOutside->read();
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
}

byte* FreezerMote::sensorData() {
  return (byte*) &_sensorData.raw;
}

void FreezerMote::setupPorts() {
  DDRD  = B10000011;  // set Arduino pins 2 to 7 as inputs, leaves 0 & 1 (RX & TX) as is
  DDRB  = B00000000;  // set pins 8 to 13 as inputs
  DDRC  = B11110000;  // set pins A0 to A3 as inputs
  PORTD = B01110100;  // enable pullups on pins 2 to 7, leave pins 0 and 1 alone
  PORTB = B11111111;  // enable pullups on pins 8 to 13
  PORTC = B11110000;  // enable pullups on pins 8 to 13
}

#endif
