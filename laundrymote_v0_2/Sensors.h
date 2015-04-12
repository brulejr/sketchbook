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
  byte waterLeak;   // 0 or 1
};

class Sensors {
  public:
    Sensors();
    void measure();
    void report(SensorData* sensorData);
    static int readWaterLeak(byte pin);
  private:
    Reading* _waterLeak;
};

#endif
