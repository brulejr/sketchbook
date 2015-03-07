/*
  Reading.cpp - Library for sensor measurement.
  Created by Jon R. Brule, January 20, 2015.
  Released into the public domain.
*/
#include "Arduino.h"
#include "Reading.h"

// constructs a standard averaged reading
Reading::Reading(byte pin, MeasureFunc mf, unsigned long period) {
    _sensorPin = pin;
    _fnMeasure = mf;
    _period = period;
    setup();
}

// constructs a averaged reading with a custom smoothing factor
Reading::Reading(byte pin, MeasureFunc mf, unsigned long period, byte smooth) {
    _sensorPin = pin;
    _fnMeasure = mf;
    _period = period;
    _smooth = smooth;
    setup();
}

// performs common constructor setup
void Reading::setup() {
    _firstTime = true;
    _lastMillis = 0;
}

//------------------------------------------------------------------------------
// loop to perform delagate measurement
//
void Reading::measure() {
    unsigned long timePassed = millis() - _lastMillis;
    if (timePassed < 0) {
        _lastMillis = millis();
    }
    if (timePassed < 0 || timePassed > _period) {
        int data = _fnMeasure(_sensorPin);
        _reading = (_smooth != NO_SMOOTHING) ? smoothedAverage(_reading, data) : data;
    }
}

//------------------------------------------------------------------------------
// returns the current reading
//
int Reading::reading() {
    return _reading;
}

//------------------------------------------------------------------------------
// perform simple smoothing as a running average
//
int Reading::smoothedAverage(int prev, int next) {
    return (_firstTime) ? next : ((_smooth - 1) * prev + next + _smooth / 2) / _smooth;
}
