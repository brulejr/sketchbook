/*
  DoorSensor.h - Sensor definition
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef DOOR_SENSOR_H
#define DOOR_SENSOR_H

#include "Arduino.h"
#include "Sensor.h"

class DoorSensor : public Sensor {
  public:
    inline DoorSensor(byte userPin) { 
      _pin = userPin; 
    }
    virtual int read(); 
  private:
    byte _pin;
};

int DoorSensor::read() {
  return digitalRead(_pin);
}

#endif
