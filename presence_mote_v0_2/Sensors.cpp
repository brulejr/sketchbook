/*
  Sensors.cpp - Sensors library
  Created 04-FEB-2017 by Jon Brule
*/
#include "Arduino.h"
#include "Sensors.h"

//------------------------------------------------------------------------------
// checks connectivity
//
Sensors::Sensors(int doorPin, int lightPin, int motionPin, MQTT* mqtt) {
  _doorPin = doorPin;
  _lightPin = lightPin;
  _motionPin = motionPin;
  _mqtt = mqtt;

  pinMode(_doorPin, INPUT_PULLUP);
  pinMode(_lightPin, INPUT);

  readDoor(true);
  readLight(true);
  readMotion(true);
}

//------------------------------------------------------------------------------
// check the current sensors
//
void Sensors::check() {
  readLight(true);
  readMotion(true);
  
  if (_stateChange) {
    _report("alert");
    _stateChange = false;
  }
  if (_doorState == HIGH) {
    if (_motionState == HIGH) {
      _report("alert");
    }
  }  
}

//------------------------------------------------------------------------------
// logic to be called by global interrupt service routine
//
void Sensors::interrupt() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200) {
    readDoor(true);
    _stateChange = true;
  }
  last_interrupt_time = interrupt_time;  
}

//------------------------------------------------------------------------------
// obtains door measurement
//
int Sensors::readDoor(bool read) {
  if (read) {
    _doorState = digitalRead(_doorPin);
  }
  return _doorState;
}

//------------------------------------------------------------------------------
// obtains light measurement
//
int Sensors::readLight(bool read) {
  if (read) {
    _lightState = analogRead(_lightPin);
  }
  return _lightState;
}

//------------------------------------------------------------------------------
// obtains motion measurement
//
int Sensors::readMotion(bool read) {
  if (read) {
    _motionState = digitalRead(_motionPin);
  }
  return _motionState;
}

//------------------------------------------------------------------------------
// publishes a sensor report
//
void Sensors::_report(char* topic) {
    const char* doorState = (_doorState == LOW ? "OPEN" : "CLOSED");
    String json = "{";
    
    json += "\"door\": \""; 
    json += (_doorState == LOW ? "OPEN" : "CLOSED"); 
    json += "\"";
    
    json += ", \"light\": \""; 
    json += _lightState; 
    json += "\"";
    
    json += ", \"motion\": \""; 
    json += (_motionState == LOW ? "NONE" : "DETECTED"); 
    json += "\"";
    
    json += "}";
    Serial.print(topic); Serial.print(": "); Serial.println(json);
    _mqtt->publish(topic, json.c_str());      
}

