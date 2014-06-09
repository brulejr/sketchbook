/*
  Freezer.h - Library for freezer sensor measurement.
  Created by Jon R. Brule, June 8, 2014.
  Released into the public domain.
*/
#ifndef Freezer_h
#define Freezer_h

#include "Arduino.h"

#define SERIAL        1

#define APIN_BATTERY  0  // data connection for battery
#define APIN_LDR      1  // data connection for photocell
#define DPIN_HALL     7  // data connection for hall effect
#define VOLTAGE       3.3

#define SMOOTHING     3


struct SensorData {
    byte msgtype;
    byte network;
    byte node;
    struct {
        byte light;   // light sensor: 0..255
        byte door;    // door sensor: 0..1
        int battery;  // battery voltage
    };
};

class Freezer
{
  public:
    Freezer(byte reportCycle);
    void measure();
  protected:
    void doMeasure();
    void doReport();
    int smoothedAverage(int prev, int next);
  private:
    byte _reportCycle;
    byte _smooth;
    byte _reportCnt;
    SensorData _reading;
    byte _firstTime;
};

#endif
