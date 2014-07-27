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

#define VOLTAGE       3.3

#define SMOOTHING     3

struct SensorData {
    byte light;    // switch sensor: 0..1
    byte battery;  // battery voltage
};

class Light
{
  public:
    Light(byte reportCycle, byte lightPin, byte battPin);
    boolean isReportReady();
    void measure();
    void off();
    void on();
    SensorData* report();
    void toggle();
  protected:
    void doMeasure();
    void doReport();
    int smoothedAverage(int prev, int next);
  private:
    byte _battPin;
    byte _lightPin;
    byte _lightState;
    byte _reportCycle;
    byte _smooth;
    byte _reportCnt;
    boolean _firstTime;
    boolean _reportReady;
    SensorData _reading, _report;
};

#endif
