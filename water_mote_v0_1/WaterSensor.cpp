/*
  WaterSensor.cpp - Encapsulates water sensor measurement.
  Created 22-JAN-2017 by Jon Brule
*/
#include "Arduino.h"
#include "WaterSensor.h"

WaterSensor::WaterSensor(byte dataPin) {
  _dataPin = dataPin;
}

//------------------------------------------------------------------------------
// measures temperature
//
boolean WaterSensor::measure() {
  return (digitalRead(_dataPin) == LOW);
}

//------------------------------------------------------------------------------
// configures sensor
//
void WaterSensor::setup() {
  pinMode(_dataPin, INPUT);
}

