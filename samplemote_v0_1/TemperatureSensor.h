/*
  TemperatureSensor.h - Sensor definition
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "Arduino.h"
#include "Sensor.h"

class TemperatureSensor : public Sensor {
  public:
    inline TemperatureSensor(byte userPin, int voltage) { 
      _pin = userPin;
      _voltage = voltage;
    }
    virtual int read(); 
  private:
    byte _pin;
    int _voltage;
};

int TemperatureSensor::read() {
  int tempRaw = analogRead(_pin);
  float tempVolts = (((float)tempRaw / 1024) * _voltage);
  return int((tempVolts - 0.5) / 0.01);
}

#endif
