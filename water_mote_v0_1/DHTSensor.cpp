/*
  DHTSensor.cpp - Encapsulates temperature / humidity measurement.
  Created 22-JAN-2017 by Jon Brule
*/
#include "Arduino.h"
#include "DHTSensor.h"

DHTSensor::DHTSensor(byte dataPin) {
  _dataPin = dataPin;
  _ready = false;
}

//------------------------------------------------------------------------------
// measures temperature
//
float DHTSensor::humidity() {
  if (_ready) {
    sensors_event_t event;
    _dht->humidity().getEvent(&event);
    return event.relative_humidity;
  } else {
    return MISSING_READING;
  } 
}

//------------------------------------------------------------------------------
// configures sensor
//
void DHTSensor::setup() {
    _dht = new DHT_Unified(_dataPin, DHTTYPE);
    _dht->begin();
    sensor_t sensor;
    _dht->temperature().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Temperature");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
    _dht->humidity().getSensor(&sensor);
    Serial.println("Humidity");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
    Serial.println("------------------------------------");
    _ready = true;
}

//------------------------------------------------------------------------------
// measures temperature
//
float DHTSensor::temperature() {
  if (_ready) {
    sensors_event_t event;
    _dht->temperature().getEvent(&event);
    return event.temperature;
  } else {
    return MISSING_READING;
  }
}

