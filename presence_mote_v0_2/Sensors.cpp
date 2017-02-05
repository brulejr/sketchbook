/*
  Sensors.cpp - Sensors library
  Created 04-FEB-2017 by Jon Brule
*/
#include "Arduino.h"
#include "Sensors.h"

//------------------------------------------------------------------------------
// checks connectivity
//
Sensors::Sensors(int doorPin, int lightPin, int motionPin) {
  _doorPin = doorPin;
  _lightPin = lightPin;
  _motionPin = motionPin;

  pinMode(_doorPin, INPUT_PULLUP);
  pinMode(_lightPin, INPUT);

  readDoor(true);
  readLight(true);
  readMotion(true);
}

//------------------------------------------------------------------------------
// obtains door measurement
//
int Sensors::readDoor(bool read) {
  if (read) {
    _doorState = digitalRead(_doorPin);
  }
  return _doorState;
}

//------------------------------------------------------------------------------
// obtains light measurement
//
int Sensors::readLight(bool read) {
  if (read) {
    _lightState = analogRead(_lightPin);
  }
  return _lightState;
}

//------------------------------------------------------------------------------
// obtains motion measurement
//
int Sensors::readMotion(bool read) {
  if (read) {
    _motionState = digitalRead(_motionPin);
  }
  return _motionState;
}

