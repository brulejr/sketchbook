/*
  DoorSensor.h - Sensor definition
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef DOOR_SENSOR_H
#define DOOR_SENSOR_H

#include "Arduino.h"
#include "Sensor.h"

#define NC_SWITCH false
#define NO_SWITCH true

class DoorSensor : public Sensor {
  public:
    inline DoorSensor(byte userPin, bool mode = NO_SWITCH) { 
      _pin = userPin;
      _mode = mode;
      pinMode(_pin, INPUT);
    }
    virtual int read(); 
  private:
    byte _pin;
    boolean _mode;
};

int DoorSensor::read() {
  bool value = digitalRead(_pin);
  return (_mode) ? value : !value;
}

#endif
