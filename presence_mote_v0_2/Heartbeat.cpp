/*
  Sensors.cpp - Heartbeat library
  Created 18-FEB-2017 by Jon Brule
*/
#include "Arduino.h"
#include "Heartbeat.h"

const long Heartbeat::_heartbeatConfig[] = { 50, 100, 150, 2000 };

//------------------------------------------------------------------------------
// constructor
//
Heartbeat::Heartbeat(int pin, float tempo) {
  _pin = pin;
  _tempo = tempo;
}

//------------------------------------------------------------------------------
// performs the heartbeat
//
void Heartbeat::beat() {
  unsigned long currentTime = millis();
  if ((currentTime - _lastTime) > (long)(_heartbeatConfig[_index] * _tempo)) {
    _index++;
    if (_index > 3) _index = 0;

    if ((_index % 2) == 0) {     // modulo 2 operator will be true on even counts
      digitalWrite(_pin, HIGH);
    } else {
      digitalWrite(_pin, LOW);
    }
    _lastTime = currentTime;
  }
 }
 
//------------------------------------------------------------------------------
// configures the heartbeat
//
void Heartbeat::setup() {
  pinMode(_pin, OUTPUT);
}
