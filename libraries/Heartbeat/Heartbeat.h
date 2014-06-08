/*
  heartbeat.h - Library for heartbeat.
  Created by Jon R. Brule, June 7, 2014.
  Released into the public domain.
*/
#ifndef Heartbeat_h
#define Heartbeat_h

#include "Arduino.h"

#define DEFAULT_LED_PIN   9
#define DEFAULT_RATE      25
#define DEFAULT_PMW       255

#define round(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

class Heartbeat
{
  public:
    Heartbeat(int pin);
    Heartbeat(int pin, int rate, int pmw);
    void pulse();
  private:
    int _pin;
    int _rate;
    int _pmw;
};

#endif
