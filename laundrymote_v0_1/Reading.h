/*
  Reading.h - Library for sensor measurement.
  Created by Jon R. Brule, January 20, 2015.
  Released into the public domain.
*/
#ifndef Reading_h
#define Reading_h

#include "Arduino.h"

#define DEFAULT_SMOOTHING  3
#define NO_SMOOTHING       0

typedef int (* MeasureFunc) (byte pin);

class Reading {
  public:
    Reading(byte pin, MeasureFunc mf, unsigned long period);
    Reading(byte pin, MeasureFunc mf, unsigned long period, byte smooth);
    void measure();
    int reading();
  protected:
    void setup();
    int smoothedAverage(int prev, int next);
  private:
    byte _sensorPin;
    byte _smooth = DEFAULT_SMOOTHING;
    MeasureFunc _fnMeasure;
    boolean _firstTime;
    int _reading;
    unsigned long _period;
    unsigned long _lastMillis;
};

#endif
