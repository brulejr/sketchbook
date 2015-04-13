/*
  Sensors.h - Library for sensor measurement.
  Created by Jon R. Brule, March 5, 2015.
  Released into the public domain.
*/
#ifndef Sensors_h
#define Sensors_h

#include "Arduino.h"
#include "Reading.h"

typedef struct SensorData {
  byte tempInside;   // temperature sensor: C * 10
  byte tempOutside;  // temperature sensor: C * 10
  byte light;        // light sensor: 0..255
  byte door;         // 0 or 1
};

class Sensors {
  public:
    Sensors();
    void measure();
    void report(SensorData* sensorData);
  private:
    Reading* _door;
    Reading* _light;
    Reading* _tempInside;
    Reading* _tempOutside;
    static int readDoor(byte pin);
    static int readLight(byte pin);
    static int readTemp(byte pin);
};

#endif
