/*
  Sensors.cpp - Sensors library
  Created 18-FEB-2017 by Jon Brule
*/
#include "Arduino.h"
#include "Sensors.h"

//------------------------------------------------------------------------------
// checks connectivity
//
Sensors::Sensors(
  int doorPin, 
  int dhtPin, 
  int lightPin, 
  int motionPin, 
  int waterPin, 
  MQTT* mqtt, 
  int interruptTimer
) {
  _doorPin = doorPin;
  _dhtPin = dhtPin;
  _lightPin = lightPin;
  _motionPin = motionPin;
  _waterPin = waterPin;
  _mqtt = mqtt;
  _interruptTimer = interruptTimer;
}

//------------------------------------------------------------------------------
// check the current sensors for alerts
//
void Sensors::checkForAlerts() {
  if (_stateChange) {
    if (_waterPresent) {
        _report("alert/water");
    } else if (_doorOpen) {
        _report("alert/door");
    } else if (_motionPresent) {
        _report("alert/motion");
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
  if (interrupt_time - last_interrupt_time > _interruptTimer) {
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
  if (interrupt_time - last_interrupt_time > _interruptTimer) {
    boolean motionPresent = _readMotion();
    if (motionPresent != _motionPresent) {
      _motionPresent = motionPresent;
      _stateChange = true;
    }
  }
  last_interrupt_time = interrupt_time;  
}

//------------------------------------------------------------------------------
// logic to be called by global interrupt service routine water state changes
//
void Sensors::handleWaterInterrupt() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > _interruptTimer) {
    _waterPresent = _readWater();
    _stateChange = true;
  }
  last_interrupt_time = interrupt_time;  
}
//------------------------------------------------------------------------------
// 
//
void Sensors::measure() {
  Serial.print("measuring...");
  _doorOpen = _readDoor();
  _humidity = _readHumidity();
  _lightState = _readLight();
  _motionPresent = _readMotion();
  _temperature = _readTemperature();
  _waterPresent = _readWater();
  Serial.println("DONE");
  _report("measurement");
}

//------------------------------------------------------------------------------
// performs any sensor configuration
//
void Sensors::setup() {
  
  pinMode(_doorPin, INPUT_PULLUP);
  pinMode(_lightPin, INPUT);
  pinMode(_motionPin, INPUT);
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
  if (!isnan(event.relative_humidity)) {
    return event.relative_humidity;
  } else {
    return BAD_READING;
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
  if (!isnan(event.temperature)) {
    return event.temperature;
  } else {
    return BAD_READING;
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
  Serial.print("reporting... ");
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
  _mqtt->publish(topic, json.c_str());
  Serial.println("DONE");
  Serial.print(topic); Serial.print(": "); Serial.println(json);
}

