/*
  Light.cpp - Library for switch sensor measurement.
  Created by Jon R. Brule, June 8, 2014.
  Released into the public domain.
*/
#include "Arduino.h"
#include "Light.h"

Light::Light(byte reportCycle)
{
    _reportCycle = reportCycle;

    _smooth = SMOOTHING;
    _firstTime = true;
    _reportReady = false;
    
    // configure pins
    pinMode(DPIN_LIGHT, OUTPUT);
}

//------------------------------------------------------------------------------
// loop to perform delagate measurement
//
void Light::measure() {
    doMeasure();
    if (++_reportCnt >= _reportCycle) {
        doReport();
        _reportCnt = 0;
    }
}

//------------------------------------------------------------------------------
// perform simple smoothing as a running average
//
int Light::smoothedAverage(int prev, int next) {
    return (_firstTime) ? next : ((_smooth - 1) * prev + next + _smooth / 2) / _smooth;
}

//------------------------------------------------------------------------------
void Light::doMeasure() {
    #if SERIAL
        Serial.println("doMeasure()");
    #endif

    // read switch status
    _reading.light = digitalRead(DPIN_LIGHT);

    // read battery voltage
    int val = analogRead(APIN_BATTERY);
    int battery = int(((val/255) * VOLTAGE) * 10);
    _reading.battery = smoothedAverage(_reading.battery, battery);
    
}

//------------------------------------------------------------------------------
void Light::doReport() {
    #if SERIAL
        Serial.print("FREEZER ");
        Serial.print((int) _reading.light);
        Serial.print(' ');
        Serial.print((int) _reading.battery);
        Serial.println();
    #endif

    memcpy(&_report, &_reading, sizeof(_reading));
    _reportReady = true;
}

//------------------------------------------------------------------------------
boolean Light::isReportReady() {
    return _reportReady;
}

//------------------------------------------------------------------------------
SensorData* Light::report() {
    _reportReady = false;
    return &_report;
}
