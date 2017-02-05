/*
  Sensors.cpp - Sensors library
  Created 04-FEB-2017 by Jon Brule
*/
#include "Arduino.h"
#include "Sensors.h"

//------------------------------------------------------------------------------
// checks connectivity
//
Sensors::Sensors(int doorPin, int lightPin, int motionPin, int waterPin, MQTT* mqtt) {
  _doorPin = doorPin;
  _lightPin = lightPin;
  _motionPin = motionPin;
  _waterPin = waterPin;
  _mqtt = mqtt;

  pinMode(_doorPin, INPUT_PULLUP);
  pinMode(_lightPin, INPUT);
  pinMode(_waterPin, INPUT);

  readDoor(true);
  readLight(true);
  readMotion(true);
  readWater(true);
}

//------------------------------------------------------------------------------
// check the current sensors
//
void Sensors::check() {
  readLight(true);
  readMotion(true);
  readWater(true);
  
  if (_stateChange) {
    _report("alert");
    _stateChange = false;
  }
  if (!_doorOpen) {
    if (_motionPresent) {
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
// obtains water measurement
//
boolean Sensors::readWater(boolean read) {
  if (read) {
    _waterPresent = (digitalRead(_waterPin) == LOW);
  }
  return _waterPresent;
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
    
    json += ", \"water\": \""; 
    json += (_waterPresent ? "PRESENT" : "NOT PRESENT"); 
    json += "\"";
    
    json += "}";
    Serial.print(topic); Serial.print(": "); Serial.println(json);
    _mqtt->publish(topic, json.c_str());      
}

