/*
  BatterySensor.h - Sensor definition
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef BATTERY_SENSOR_H
#define BATTERY_SENSOR_H

#include "Arduino.h"
#include "Sensor.h"

class BatterySensor : public Sensor {
  public:
    inline BatterySensor(byte userPin) { 
      _pin = userPin; 
    }
    virtual int read(); 
  private:
    byte _pin;
};

int BatterySensor::read() {
  return map(analogRead(_pin), 0, 1023, 0, 255);
}

#endif
