/*
  WaterSensor.h - Encapsulates water sensor measurement.
  Created 22-JAN-2017 by Jon Brule
*/
#ifndef WaterSensor_h
#define WaterSensor_h

#include "Arduino.h"

class WaterSensor
{
  public:
    WaterSensor(byte dataPin);
    boolean measure();
    void setup();
  private:
    byte _dataPin;
};

#endif
