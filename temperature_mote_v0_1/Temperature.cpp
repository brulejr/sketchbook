/*
  Temperature.cpp - Library for temperature sensor measurement.
  Created by Jon R. Brule, January 12, 2015.
  Released into the public domain.
*/
#include "Arduino.h"
#include "Temperature.h"

Temperature::Temperature(byte reportCycle, byte temperaturePin, byte battPin)
{
    _reportCycle = reportCycle;
    _temperaturePin = temperaturePin;
    _battPin = battPin;

    _smooth = SMOOTHING;
    _firstTime = true;
    _reportReady = false;
    
    // configure temperature
    pinMode(_temperaturePin, OUTPUT);
}

//------------------------------------------------------------------------------
// loop to perform delagate measurement
//
void Temperature::measure() {
    doMeasure();
    if (++_reportCnt >= _reportCycle) {
        doReport();
        _reportCnt = 0;
    }
}

//------------------------------------------------------------------------------
// perform simple smoothing as a running average
//
int Temperature::smoothedAverage(int prev, int next) {
    return (_firstTime) ? next : ((_smooth - 1) * prev + next + _smooth / 2) / _smooth;
}

//------------------------------------------------------------------------------
void Temperature::doMeasure() {
    #if SERIAL
        Serial.println("doMeasure()");
    #endif

    // read temperature
    int tempRaw = analogRead(_temperaturePin);
    float tempVolts = (((float)tempRaw / 1024) * 3.3);
    int tempInC = int((tempVolts - 0.5) / 0.01);
    _reading.tempInC = smoothedAverage(_reading.tempInC, tempInC);

    // read battery voltage
    int val = analogRead(_battPin);
    int battery = int(((val/255) * VOLTAGE) * 10);
    _reading.battery = smoothedAverage(_reading.battery, battery);
    
}

//------------------------------------------------------------------------------
void Temperature::doReport() {
    #if SERIAL
        Serial.print("TEMPERATURE ");
        Serial.print((int) _reading.tempInC);
        Serial.print(' ');
        Serial.print((int) _reading.battery);
        Serial.println();
    #endif

    memcpy(&_report, &_reading, sizeof(_reading));
    _reportReady = true;
}

//------------------------------------------------------------------------------
boolean Temperature::isReportReady() {
    return _reportReady;
}

//------------------------------------------------------------------------------
SensorData* Temperature::report() {
    _reportReady = false;
    return &_report;
}
