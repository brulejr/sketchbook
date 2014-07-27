/*
  Light.cpp - Library for switch sensor measurement.
  Created by Jon R. Brule, June 8, 2014.
  Released into the public domain.
*/
#include "Arduino.h"
#include "Light.h"

Light::Light(byte reportCycle, byte lightPin, byte battPin)
{
    _reportCycle = reportCycle;
    _lightPin = lightPin;
    _battPin = battPin;

    _smooth = SMOOTHING;
    _firstTime = true;
    _reportReady = false;
    
    // configure light
    _lightState = LOW;
    pinMode(_lightPin, OUTPUT);
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
// turn the light off
//
void Light::off() {
   _lightState = LOW;
   digitalWrite(_lightPin, _lightState); 
}

//------------------------------------------------------------------------------
// turn the light on
//
void Light::on() {
   _lightState = HIGH;
   digitalWrite(_lightPin, _lightState); 
}

//------------------------------------------------------------------------------
// perform simple smoothing as a running average
//
int Light::smoothedAverage(int prev, int next) {
    return (_firstTime) ? next : ((_smooth - 1) * prev + next + _smooth / 2) / _smooth;
}

//------------------------------------------------------------------------------
// toggle the light state
//
void Light::toggle() {
   _lightState = (_lightState == LOW) ? HIGH : LOW;
   digitalWrite(_lightPin, _lightState); 
}

//------------------------------------------------------------------------------
void Light::doMeasure() {
    #if SERIAL
        Serial.println("doMeasure()");
    #endif

    // read switch status
    _reading.light = digitalRead(_lightPin);

    // read battery voltage
    int val = analogRead(_battPin);
    int battery = int(((val/255) * VOLTAGE) * 10);
    _reading.battery = smoothedAverage(_reading.battery, battery);
    
}

//------------------------------------------------------------------------------
void Light::doReport() {
    #if SERIAL
        Serial.print("LIGHT ");
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
