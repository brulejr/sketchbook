/*
  Freezer.cpp - Library for freezer sensor measurement.
  Created by Jon R. Brule, June 8, 2014.
  Released into the public domain.
*/
#include "Arduino.h"
#include "Freezer.h"

Freezer::Freezer(byte reportCycle)
{
    _reportCycle = reportCycle;

    _smooth = SMOOTHING;
    _firstTime = true;
    _reportReady = false;
    
    // configure pins
    pinMode(APIN_LDR, INPUT);
    pinMode(DPIN_HALL, INPUT);
}

//------------------------------------------------------------------------------
// loop to perform delagate measurement
//
void Freezer::measure() {
    doMeasure();
    if (++_reportCnt >= _reportCycle) {
        doReport();
        _reportCnt = 0;
    }
}

//------------------------------------------------------------------------------
// perform simple smoothing as a running average
//
int Freezer::smoothedAverage(int prev, int next) {
    return (_firstTime) ? next : ((_smooth - 1) * prev + next + _smooth / 2) / _smooth;
}

//------------------------------------------------------------------------------
void Freezer::doMeasure() {
    #if SERIAL
        Serial.println("doMeasure()");
    #endif
    
    // read light level
    byte light = map(analogRead(APIN_LDR), 0, 1023, 0, 255);
    _reading.light = smoothedAverage(_reading.light, light);

    // read door status
    _reading.door = digitalRead(DPIN_HALL);

    // read battery voltage
    int val = analogRead(APIN_BATTERY);
    int battery = int(((val/255) * VOLTAGE) * 10);
    _reading.battery = smoothedAverage(_reading.battery, battery);
    
}

//------------------------------------------------------------------------------
void Freezer::doReport() {
    #if SERIAL
        Serial.print("FREEZER ");
        Serial.print((int) _reading.light);
        Serial.print(' ');
        Serial.print((int) _reading.door);
        Serial.print(' ');
        Serial.print((int) _reading.battery);
        Serial.println();
    #endif
    
    memset(_message.raw, 0, sizeof(_message.raw));
    
    memcpy(&_message.sensor, &_reading, sizeof(_reading));
    _message.sensor.msgtype = MSG_READING;
    _message.sensor.network = NETWORKID;
    _message.sensor.node = NODEID;
    _reportReady = true;
}

//------------------------------------------------------------------------------
boolean Freezer::isReportReady() {
    return _reportReady;
}

//------------------------------------------------------------------------------
MessageData* Freezer::report() {
    _reportReady = false;
    return &_message;
}
