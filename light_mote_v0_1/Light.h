/*
  Light.h - Library for freezer sensor measurement.
  Created by Jon R. Brule, June 8, 2014.
  Released into the public domain.
*/
#ifndef Light_h
#define Light_h

#include "Arduino.h"
#include <Event.h>

#define SERIAL        1

#define APIN_BATTERY  0  // data connection for battery
#define DPIN_LIGHT    3  // data connection for hall effect
#define VOLTAGE       3.3

#define SMOOTHING     3

struct SensorData {
    byte light;    // switch sensor: 0..1
    byte battery;  // battery voltage
};

class Light
{
  public:
    Light(byte reportCycle);
    void measure();
    boolean isReportReady();
    SensorData* report();
  protected:
    void doMeasure();
    void doReport();
    int smoothedAverage(int prev, int next);
  private:
    byte _reportCycle;
    byte _smooth;
    byte _reportCnt;
    boolean _firstTime;
    boolean _reportReady;
    SensorData _reading, _report;
};

#endif
