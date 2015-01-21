/*
  Reading.cpp - Library for sensor measurement.
  Created by Jon R. Brule, January 20, 2015.
  Released into the public domain.
*/
#include "Arduino.h"
#include "Reading.h"

// constructs a averaged reading
Reading::Reading(byte period, byte pin, byte smooth, MeasureFunc mf, ReportFunc rf) {
    _reportPeriod = period;
    _sensorPin = pin;
    _smooth = smooth;
    _fnMeasure = mf;
    _fnReport = rf;
    setup();
}

// constructs a non-averaged reading
Reading::Reading(byte period, byte pin, MeasureFunc mf, ReportFunc rf) {
    _reportPeriod = period;
    _sensorPin = pin;
    _fnMeasure = mf;
    _fnReport = rf;
    setup();
}

// performs common constructor setup
void Reading::setup() {
    _firstTime = true;
    pinMode(_sensorPin, OUTPUT);
}

//------------------------------------------------------------------------------
// loop to perform delagate measurement
//
void Reading::measure() {
    int data = _fnMeasure(_sensorPin);
    _reading = (_smooth != NO_SMOOTHING) ? smoothedAverage(_reading, data) : data;
    if (++_reportCnt >= _reportPeriod) {
        _fnReport(_reading);
        _reportCnt = 0;
    }
}

//------------------------------------------------------------------------------
// perform simple smoothing as a running average
//
int Reading::smoothedAverage(int prev, int next) {
    return (_firstTime) ? next : ((_smooth - 1) * prev + next + _smooth / 2) / _smooth;
}
