/*
  Reading.h - Library for sensor measurement.
  Created by Jon R. Brule, January 20, 2015.
  Released into the public domain.
*/
#ifndef Reading_h
#define Reading_h

#include "Arduino.h"

#define NO_SMOOTHING      0

typedef int (* MeasureFunc) (byte pin);
typedef void (* ReportFunc) (int reading);

class Reading {
  public:
    Reading(byte period, byte pin, byte smooth, MeasureFunc mf, ReportFunc rf);
    void measure();
  protected:
    int smoothedAverage(int prev, int next);
  private:
    byte _reportPeriod;
    byte _sensorPin;
    byte _smooth;
    MeasureFunc _fnMeasure;
    ReportFunc _fnReport;
    byte _reportCnt;
    boolean _firstTime;
    int _reading;
};

#endif
