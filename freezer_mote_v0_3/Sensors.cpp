/*
  Sensors.cpp - Logic for sensor readings
  Created by Jon R. Brule, April 13, 2015.
  Released into the public domain.
*/
#include "Sensors.h"
#include "Reading.h"

#define DEBUG               1

#define BATTERY_PIN          A0
#define BATTERY_PERIOD       5000 
#define DOOR_PIN             3
#define DOOR_PERIOD          500
#define LIGHT_PIN            A1
#define LIGHT_PERIOD         500
#define TEMP_INSIDE_PIN      A2
#define TEMP_INSIDE_PERIOD   500
#define TEMP_OUTSIDE_PIN     A3
#define TEMP_OUTSIDE_PERIOD  500

#define VOLTAGE              3.3
#define SUPPLY_VOLTAGE       6

//-----------------------------------------------------------------------------
Sensors::Sensors() {
  #if DEBUG
    Serial.print("setup sensors...");
  #endif
  
  pinMode(DOOR_PIN, INPUT);
  
  _battery = new Reading(BATTERY_PIN, Sensors::readBattery, BATTERY_PERIOD);
  _door = new Reading(DOOR_PIN, Sensors::readDoor, DOOR_PERIOD);
  _light = new Reading(LIGHT_PIN, Sensors::readLight, LIGHT_PERIOD);
  _tempInside = new Reading(TEMP_INSIDE_PIN, Sensors::readTemp, TEMP_INSIDE_PERIOD);
  _tempOutside = new Reading(TEMP_OUTSIDE_PIN, Sensors::readTemp, TEMP_OUTSIDE_PERIOD);
  
  #if DEBUG
    Serial.println("ok!");
  #endif
}

//-----------------------------------------------------------------------------
void Sensors::measure() {
  _battery->measure();
  _door->measure();
  _light->measure();
  _tempInside->measure();
  _tempOutside->measure();
}

//-----------------------------------------------------------------------------
void Sensors::report(SensorData* sensorData) {
  sensorData->battery = _battery->reading();
  sensorData->door = _door->reading();
  sensorData->light = _light->reading();
  sensorData->tempInside = _tempInside->reading();
  sensorData->tempOutside = _tempOutside->reading();
  #if DEBUG
    Serial.print("Sensors<battery = ");
    Serial.print(sensorData->battery);
    Serial.print(", tempInside = ");    
    Serial.print(sensorData->tempInside);
    Serial.print(", tempOutside = ");    
    Serial.print(sensorData->tempOutside);
    Serial.print(", light = ");    
    Serial.print(sensorData->light);
    Serial.print(", door = ");    
    Serial.print(sensorData->door);
    Serial.println(">");
    Serial.flush();
  #endif
}

//-----------------------------------------------------------------------------
int Sensors::readBattery(byte pin) {
  return map(analogRead(pin), 0, 1023, 0, 255);
}

//-----------------------------------------------------------------------------
int Sensors::readDoor(byte pin) {
  return digitalRead(pin);
}

//-----------------------------------------------------------------------------
int Sensors::readLight(byte pin) {
  return map(analogRead(pin), 0, 1023, 0, 255);
}

//-----------------------------------------------------------------------------
int Sensors::readTemp(byte pin) {
  int tempRaw = analogRead(pin);
  float tempVolts = (((float)tempRaw / 1024) * VOLTAGE);
  return int((tempVolts - 0.5) / 0.01);
}


