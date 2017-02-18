/*
  Sensors.cpp - Sensors library
  Created 04-FEB-2017 by Jon Brule
*/
#include "Arduino.h"
#include "Sensors.h"

//------------------------------------------------------------------------------
// checks connectivity
//
Sensors::Sensors(int doorPin, int dhtPin, int lightPin, int motionPin, int waterPin, MQTT* mqtt) {
  _doorPin = doorPin;
  _dhtPin = dhtPin;
  _lightPin = lightPin;
  _motionPin = motionPin;
  _waterPin = waterPin;
  _mqtt = mqtt;
}

//------------------------------------------------------------------------------
// check the current sensors for alerts
//
void Sensors::checkForAlerts() {
  if (_stateChange) {
    _report("alert");
  }
  if (!_doorOpen) {
    if (_motionPresent) {
      _report("alert");
    }
  }
  _stateChange = false;
}

//------------------------------------------------------------------------------
// logic to be called by global interrupt service routine door state changes
//
void Sensors::handleDoorInterrupt() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200) {
    readDoor(true);
    _stateChange = true;
  }
  last_interrupt_time = interrupt_time;  
}

//------------------------------------------------------------------------------
// logic to be called by global interrupt service routine when motion detected
//
void Sensors::handleMotionInterrupt() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200) {
    readMotion(true);
    _stateChange = true;
  }
  last_interrupt_time = interrupt_time;  
}

//------------------------------------------------------------------------------
// 
//
void Sensors::measure() {
  readDoor(true);
  readHumidity(true);
  readLight(true);
  readMotion(true);
  readWater(true);
  readTemperature(true);
  _report("measurement");
}

//------------------------------------------------------------------------------
// obtains the humidity
//
float Sensors::readHumidity(boolean read) {
  if (read) {
    sensors_event_t event;
    _dht->humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      _humidity = event.relative_humidity;
    }
  }
  return _humidity;  
}

//------------------------------------------------------------------------------
// obtains door measurement
//
boolean Sensors::readDoor(boolean read) {
  if (read) {
    _doorOpen = (digitalRead(_doorPin) == LOW);
  }
  return _doorOpen;
}

//------------------------------------------------------------------------------
// obtains light measurement
//
int Sensors::readLight(boolean read) {
  if (read) {
    _lightState = analogRead(_lightPin);
  }
  return _lightState;
}

//------------------------------------------------------------------------------
// obtains motion measurement
//
boolean Sensors::readMotion(boolean read) {
  if (read) {
    _motionPresent = (digitalRead(_motionPin) == HIGH);
  }
  return _motionPresent;
}

//------------------------------------------------------------------------------
// obtains the temperature
//
float Sensors::readTemperature(boolean read) {
  if (read) {
    sensors_event_t event;
    _dht->temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      _temperature = event.temperature;
    }
  }
  return _temperature;
}

//------------------------------------------------------------------------------
// obtains water measurement
//
boolean Sensors::readWater(boolean read) {
  if (read) {
    _waterPresent = (digitalRead(_waterPin) == LOW);
  }
  return _waterPresent;
}

//------------------------------------------------------------------------------
// performs any sensor configuration
//
void Sensors::setup() {
  
  pinMode(_doorPin, INPUT_PULLUP);
  pinMode(_lightPin, INPUT);
  pinMode(_waterPin, INPUT);

  _dht = new DHT_Unified(_dhtPin, DHTTYPE);
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

  readDoor(true);
  readLight(true);
  readMotion(true);
  readWater(true);
}

//------------------------------------------------------------------------------
// publishes a sensor report
//
void Sensors::_report(char* topic) {
    String json = "{";
    
    json += "\"door\": \""; 
    json += (_doorOpen ? "OPEN" : "CLOSED"); 
    json += "\"";
    
    json += ", \"light\": \""; 
    json += _lightState; 
    json += "\"";
    
    json += ", \"motion\": \""; 
    json += (_motionPresent ? "DETECTED" : "NONE"); 
    json += "\"";
    
    json += ", \"temperature\": \""; 
    json += _temperature; 
    json += "\"";
    
    json += ", \"water\": \""; 
    json += (_waterPresent ? "PRESENT" : "NOT PRESENT"); 
    json += "\"";
    
    json += "}";
    Serial.print(topic); Serial.print(": "); Serial.println(json);
    _mqtt->publish(topic, json.c_str());      
}

