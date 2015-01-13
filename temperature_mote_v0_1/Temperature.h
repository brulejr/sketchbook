/*
  Temperature.h - Library for temperature sensor measurement.
  Created by Jon R. Brule, January 12, 2015.
  Released into the public domain.
*/
#ifndef Temperature_h
#define Temperature_h

#include "Arduino.h"
#include <Message.h>

#define SERIAL        1

#define VOLTAGE       3.3

#define SMOOTHING     3

struct SensorData {
    byte tempInC;   // temperature sensor: C * 10
    byte battery;   // battery voltage
};

class Temperature
{
  public:
    Temperature(byte reportCycle, byte lightPin, byte battPin);
    boolean isReportReady();
    void measure();
    SensorData* report();
  protected:
    void doMeasure();
    void doReport();
    int smoothedAverage(int prev, int next);
  private:
    byte _battPin;
    byte _temperaturePin;
    byte _reportCycle;
    byte _smooth;
    byte _reportCnt;
    boolean _firstTime;
    boolean _reportReady;
    SensorData _reading, _report;
};

#endif
