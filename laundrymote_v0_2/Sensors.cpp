/*
  Sensors.cpp - Logic for sensor readings
  Created by Jon R. Brule, March 5, 2015.
  Released into the public domain.
*/
#include "Sensors.h"
#include "Reading.h"

#define DEBUG               1

#define WATER_LEAK_PIN      3
#define WATER_LEAK_PERIOD   500

#define VOLTAGE             3.3

//-----------------------------------------------------------------------------
Sensors::Sensors() {
  #if DEBUG
    Serial.print("setup sensors...");
  #endif
  
  pinMode(WATER_LEAK_PIN, INPUT);
  
  _waterLeak = new Reading(WATER_LEAK_PIN, Sensors::readWaterLeak, WATER_LEAK_PERIOD);
  
  #if DEBUG
    Serial.println("ok!");
  #endif
}

//-----------------------------------------------------------------------------
void Sensors::measure() {
  _waterLeak->measure();
}

//-----------------------------------------------------------------------------
void Sensors::report(SensorData* sensorData) {
  sensorData->waterLeak = _waterLeak->reading();
  #if DEBUG
    Serial.print("Sensors<waterLeak = ");
    Serial.print(sensorData->waterLeak);
    Serial.println(">");
    Serial.flush();
  #endif
}

//-----------------------------------------------------------------------------
int Sensors::readWaterLeak(byte pin) {
  return digitalRead(pin);
}


