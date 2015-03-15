/*
  Sensors.cpp - Logic for sensor readings
  Created by Jon R. Brule, March 5, 2015.
  Released into the public domain.
*/
#include "Sensors.h"

#define DEBUG               1

#define BATTERY_PIN         A0
#define BATTERY_PERIOD      5000 
#define LIGHT_PIN           A1
#define LIGHT_PERIOD        5000
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
  
  _battery = new Reading(BATTERY_PIN, Sensors::readBattery, BATTERY_PERIOD);
  _light = new Reading(LIGHT_PIN, Sensors::readLight, LIGHT_PERIOD);
  _temperature = new Reading(TEMPERATURE_PIN, Sensors::readTemperature, TEMPERATURE_PERIOD);
  _waterLeak = new Reading(WATER_LEAK_PIN, Sensors::readWaterLeak, WATER_LEAK_PERIOD);
  
  #if DEBUG
    Serial.println("ok!");
  #endif
}

//-----------------------------------------------------------------------------
void Sensors::measure() {
    _battery->measure();
    _light->measure();
    _temperature->measure();
    _waterLeak->measure();
}

//-----------------------------------------------------------------------------
void Sensors::report(SensorData* sensorData) {
    sensorData->battery = _battery->reading();
    sensorData->light = _light->reading();
    sensorData->temperature = _temperature->reading();
    sensorData->waterLeak = _waterLeak->reading();
    #if DEBUG
        Serial.print("Sensors<battery = ");
        Serial.print(sensorData->battery);
        Serial.print(", light = ");
        Serial.print(sensorData->light);
        Serial.print(", temperature = ");
        Serial.print(sensorData->temperature);
        Serial.print(", waterLeak = ");
        Serial.print(sensorData->waterLeak);
        Serial.println(">");
    #endif
}

//-----------------------------------------------------------------------------
int Sensors::readBattery(byte pin) {
    int raw = analogRead(pin);
    return int((((float) raw / 255) * VOLTAGE) * 10);
}

//-----------------------------------------------------------------------------
int Sensors::readLight(byte pin) {
    int raw = analogRead(pin);
    return 255 - int(255 * ((float)raw / 1024));
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


