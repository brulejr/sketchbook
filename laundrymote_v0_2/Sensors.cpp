/*
  Sensors.cpp - Logic for sensor readings
  Created by Jon R. Brule, March 5, 2015.
  Released into the public domain.
*/
#include "Sensors.h"
#include "Reading.h"

#define DEBUG               1

#define TEMPERATURE_PIN     A2
#define TEMPERATURE_PERIOD  500
#define WATER_LEAK_PIN      3
#define WATER_LEAK_PERIOD   500

#define VOLTAGE             3.3

//-----------------------------------------------------------------------------
Sensors::Sensors() {
  #if DEBUG
    Serial.print("setup sensors...");
  #endif
  
  pinMode(WATER_LEAK_PIN, INPUT);
  
  _temperature = new Reading(TEMPERATURE_PIN, Sensors::readTemperature, TEMPERATURE_PERIOD);
  _waterLeak = new Reading(WATER_LEAK_PIN, Sensors::readWaterLeak, WATER_LEAK_PERIOD);
  
  #if DEBUG
    Serial.println("ok!");
  #endif
}

//-----------------------------------------------------------------------------
void Sensors::measure() {
  _temperature->measure();
  _waterLeak->measure();
}

//-----------------------------------------------------------------------------
void Sensors::report(SensorData* sensorData) {
  sensorData->temperature = _temperature->reading();
  sensorData->waterLeak = _waterLeak->reading();
  #if DEBUG
    Serial.print("Sensors<temperature = ");
    Serial.print(sensorData->temperature);
    Serial.print(", waterLeak = ");    
    Serial.print(sensorData->waterLeak);
    Serial.println(">");
    Serial.flush();
  #endif
}

//-----------------------------------------------------------------------------
int Sensors::readTemperature(byte pin) {
  int tempRaw = analogRead(pin);
  float tempVolts = (((float)tempRaw / 1024) * VOLTAGE);
  return int((tempVolts - 0.5) / 0.01);
}

//-----------------------------------------------------------------------------
int Sensors::readWaterLeak(byte pin) {
  return digitalRead(pin);
}


