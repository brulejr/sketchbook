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
    if (_doorOpen || _motionPresent) {
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
    _doorOpen = _readDoor();
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
    boolean motionPresent = _readMotion();
    if (motionPresent != _motionPresent) {
      _motionPresent = motionPresent;
      _stateChange = true;
    }
  }
  last_interrupt_time = interrupt_time;  
}

//------------------------------------------------------------------------------
// 
//
void Sensors::measure() {
  _doorOpen = _readDoor();
  _humidity = _readHumidity();
  _lightState = _readLight();
  _motionPresent = _readMotion();
  _temperature = _readTemperature();
  _waterPresent = _readWater();
  _report("measurement");
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

  measure();
}

//------------------------------------------------------------------------------
// obtains door measurement
//
boolean Sensors::_readDoor() {
  return (digitalRead(_doorPin) == LOW);
}

//------------------------------------------------------------------------------
// obtains the humidity
//
float Sensors::_readHumidity() {
  sensors_event_t event;
  _dht->humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    return event.relative_humidity;
  } else {
    return 0.0;
  }
}

//------------------------------------------------------------------------------
// obtains light measurement
//
int Sensors::_readLight() {
  return analogRead(_lightPin);
}

//------------------------------------------------------------------------------
// obtains motion measurement
//
boolean Sensors::_readMotion() {
  return (digitalRead(_motionPin) == HIGH);
}

//------------------------------------------------------------------------------
// obtains the temperature
//
float Sensors::_readTemperature() {
  sensors_event_t event;
  _dht->temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    return event.temperature;
  } else {
    return 0.0;
  }
}

//------------------------------------------------------------------------------
// obtains water measurement
//
boolean Sensors::_readWater() {
  return (digitalRead(_waterPin) == LOW);
}

//------------------------------------------------------------------------------
// publishes a sensor report
//
void Sensors::_report(char* topic) {
    String json = "{";
    
    json += "\"door\":"; 
    json += _doorOpen; 
    
    json += ",\"light\":"; 
    json += _lightState; 
    
    json += ",\"motion\":"; 
    json += _motionPresent; 
    
    json += ",\"temperature\":"; 
    json += _temperature; 
    
    json += ",\"humidity\":"; 
    json += _humidity; 
    
    json += ",\"water\":"; 
    json += _waterPresent; 
    
    json += "}";
    Serial.print(topic); Serial.print(": "); Serial.println(json);
    _mqtt->publish(topic, json.c_str());      
}

