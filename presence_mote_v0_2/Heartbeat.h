/*
  Heartbeat.h - Provides an LED heartbeat
  Created 18-FEB-2017 by Jon Brule
*/
#ifndef Heartbeat_h
#define Heartbeat_h

#include <Arduino.h>

class Heartbeat {
  public:
    Heartbeat(int pin, float tempo);
    void beat();
    void setup();
  private:
    const static long _heartbeatConfig[];
    int _index;
    int _pin;
    float _tempo;
    unsigned long _lastTime;
};

#endif
