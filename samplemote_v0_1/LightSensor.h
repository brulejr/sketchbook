/*
  LightSensor.h - Sensor definition
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include "Arduino.h"
#include "Sensor.h"

class LightSensor : public Sensor {
  public:
    inline LightSensor(byte userPin) { 
      _pin = userPin; 
    }
    virtual int read(); 
  private:
    byte _pin;
};

int LightSensor::read() {
  return map(analogRead(_pin), 0, 1023, 0, 255);
}

#endif
