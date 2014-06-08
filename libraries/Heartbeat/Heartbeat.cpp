/*
  heartbeat.cpp - RTOS thread library for an LED heartbeat.
  Created by Jon R. Brule, June 7, 2014.
  Released into the public domain.
*/
#include "Arduino.h"
#include <ChibiOS_AVR.h>
#include "Heartbeat.h"

Heartbeat::Heartbeat(int pin)
{
  _pin = pin;
  _rate = DEFAULT_RATE;
  _pmw = DEFAULT_PMW;
}

Heartbeat::Heartbeat(int pin, int rate, int pmw)
{
  _pin = pin;
  _rate = rate;
  _pmw = pmw;
}

//------------------------------------------------------------------------------
// RTOS thread that simulates a heartbeat to show activity
//
void Heartbeat::pulse() {
    for(int i = 0; i < _pmw; i++) {
        analogWrite(_pin, i);
        chThdSleepMilliseconds(round(((60000/_rate)*.1)/_pmw));
    }
    for (int i = _pmw; i > 0; i--){
        analogWrite(_pin, i);
        chThdSleepMilliseconds(round(((60000/_rate)*.2)/_pmw));
    }
    for(int i = 0; i < _pmw; i++) {
        analogWrite(_pin, i);
        chThdSleepMilliseconds(round(((60000/_rate)*.1)/_pmw));
    }
    for (int i = _pmw; i > 0; i--){
        analogWrite(_pin, i);
        chThdSleepMilliseconds(round(((60000/_rate)*.6)/_pmw));
    }
}

