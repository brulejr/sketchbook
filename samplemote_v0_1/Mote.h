/*
  Mote.h - Base library for sensor / control remote
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef Mote_h
#define Mote_h

#include <Arduino.h>
#include <EEPROM.h>
#include <LowPower.h>
#include <Message.h>
#include <RFM69.h>
#include <SPI.h>

typedef struct MoteConfig {
  byte rfNodeId = 2;
  byte rfNetworkId= 99;
  byte rfGatewayId = 1;
  byte rfFrequency = RF69_915MHZ;
  
  byte intStatusLedPin = 9;
  byte extStatusLedPin = 7;
  
  byte loopMultiplier = 30;
  byte waitPeriod = 2;
};

class Mote  {
  public:
    Mote(const char* name, const char* version, MoteConfig* config, bool init);
    void setup();
    void loop();  
    virtual void measure() = 0;
    
  protected:
    void blinkStatusLeds();
    virtual unsigned int calculateLedDelay() = 0;
    virtual byte calculateMessageLevel() = 0;
    period_t calculateWaitPeriod();
    void loadConfig();
    void report();
    inline virtual byte* sensorData() { return null; };
    void setupISR();
    inline virtual void setupPorts() { /*nothing*/ };
    void setupRadio();
    void setupStatusIndicator();
    void sleep();
    void storeConfig();
    
    Message _outbound;
    RFM69 _radio;
    
    MoteConfig* _config;
    
    static void wakeup();
    static volatile bool _intr;
    static volatile unsigned long _lastIntrTime;
    
  private:
    const char* _name;
    const char* _version;
        
    bool _init;
};

volatile bool Mote::_intr = false;
volatile unsigned long Mote::_lastIntrTime = 0;


//-----------------------------------------------------------------------------
// API Methods
//-----------------------------------------------------------------------------
Mote::Mote(const char* name, const char* version, MoteConfig* config, bool init) {
  _name = name;
  _version = version;
  _config = config;
  _init = init;
}

void Mote::setup() {
  Serial.print("\n[");
  Serial.print(_name);
  Serial.print(" - ");
  Serial.print(_version);
  Serial.println("]");
  delay(100);
  
  loadConfig();
  
  Serial.print("setup ports...");
  setupPorts();
  Serial.println("ok!");
  
  Serial.print("setup status indicators...");
  setupStatusIndicator();
  Serial.println("ok!");
  
  Serial.print("setup radio...");
  setupRadio();
  Serial.println("ok!");
  
  Serial.print("setup isr...");
  setupISR();
  Serial.println("ok!");
  
}

void Mote::loop() {
  noInterrupts();
  measure();
  report();
  interrupts();
  sleep();
}

//-----------------------------------------------------------------------------
// Support Methods
//-----------------------------------------------------------------------------
void Mote::blinkStatusLeds() {
  digitalWrite(_config->intStatusLedPin, HIGH);
  digitalWrite(_config->extStatusLedPin, HIGH);
  delay(calculateLedDelay());  
  digitalWrite(_config->intStatusLedPin, LOW);
  digitalWrite(_config->extStatusLedPin, LOW);
}

period_t Mote::calculateWaitPeriod() {
  if (_config->waitPeriod < 1) {
    return SLEEP_500MS;
  } else if (_config->waitPeriod >= 1 and _config->waitPeriod < 2) {
    return SLEEP_1S;
  } else if (_config->waitPeriod >= 2 and _config->waitPeriod < 4) {
    return SLEEP_2S;
  } else if (_config->waitPeriod >= 4 and _config->waitPeriod < 8) {
    return SLEEP_4S;
  } else {
    return SLEEP_2S;
  }
}

void Mote::loadConfig() {
  if (_init) {
    storeConfig();
  } 
  byte configSize = sizeof(*_config);
  byte* p = (byte*) _config;
  for (int i = 0; i < configSize; i++) {
    *(p + i) = EEPROM.read(i);
    Serial.print("Config[");
    Serial.print(i);
    Serial.print(" of ");
    Serial.print(configSize);
    Serial.print("] = ");
    Serial.println(EEPROM.read(i), DEC);
  } 
}

void Mote::report() {
  memset(&_outbound, 0, sizeof(_outbound));
  _outbound.msg.type = calculateMessageLevel();
  _outbound.msg.source = _config->rfNodeId;
  _outbound.msg.destination = 0;
  _outbound.msg.component = 0;
  _outbound.msg.rssi = 0;
  memcpy(&_outbound.msg.data, sensorData(), MSG_DATA_LENGTH);
  
  Serial.print("Report<");
  Serial.print(_outbound.msg.type);
  Serial.print("> from node<");
  Serial.print(_config->rfNodeId);
  Serial.print("> to gateway<");
  Serial.print(_config->rfGatewayId);
  Serial.print(">...");
  if (_radio.sendWithRetry(_config->rfGatewayId, _outbound.raw, MSG_LENGTH)) {
    Serial.println("ACK");
  } else {
    Serial.println("No ACK!");
  }
  
  blinkStatusLeds();
}

void Mote::setupISR() {
  attachInterrupt(1, wakeup, RISING);
}

void Mote::setupRadio() {
  _radio.initialize(_config->rfFrequency, _config->rfNodeId, _config->rfNetworkId);
  _radio.setHighPower();
  delay(1000);
}

void Mote::setupStatusIndicator() {
  pinMode(_config->intStatusLedPin, OUTPUT);
  pinMode(_config->extStatusLedPin, OUTPUT);
  digitalWrite(_config->intStatusLedPin, LOW);
  digitalWrite(_config->extStatusLedPin, LOW);
}

void Mote::sleep() {
  Serial.flush();
  _radio.sleep();
  for (int i = 0; i < _config->loopMultiplier; i++) {
    LowPower.powerDown(calculateWaitPeriod(), ADC_OFF, BOD_OFF);
    if (_intr) {
      _intr = false;
      break;
    }
  }
}

void Mote::storeConfig() {
  byte configSize = sizeof(*_config);
  byte* p = (byte*) _config;
  for (int i = 0; i < configSize; i++) {
    EEPROM.write(i, *(p + i));
  }
}

void Mote::wakeup() {
  _intr = true;
  _lastIntrTime = millis();
}

#endif
