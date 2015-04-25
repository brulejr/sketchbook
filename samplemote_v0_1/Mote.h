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

class Mote  {
  public:
    Mote(const char* name, const char* version, bool init);
    void setup();
    void loop();  
    virtual void measure() = 0;
    
  protected:
    void blinkStatusLeds();
    virtual unsigned int calculateLedDelay() = 0;
    virtual byte calculateMessageLevel() = 0;
    virtual byte calculateSleepMultiplier() = 0;
    inline virtual void initConfig() { /*nothing*/ };
    void loadConfig();
    void report();
    inline virtual byte* sensorData() { return null; };
    inline virtual void setupPorts() { /*nothing*/ };
    void setupRadio();
    void setupStatusIndicator();
    void sleep();
    void storeConfig();
    
    Message _outbound;
    RFM69 _radio;
    byte _alertMultiplier = 2;
    byte _loopMultiplier = 10;
    byte _rfNodeId     = 9;
    byte _rfNetworkId  = 99;
    byte _rfGatewayId  = 1;
    byte _rfFrequency  = RF69_915MHZ;
    byte _statusLedPin1 = 9;
    byte _statusLedPin2 = 7;
    
    static void wakeup();
    static bool _intr;
    static unsigned long _lastIntrTime;
};

#define ADDR_STATUS_LED_1_PIN  0
#define ADDR_STATUS_LED_2_PIN  1
#define ADDR_RF_NODE_ID        2
#define ADDR_RF_NETWORK_ID     3
#define ADDR_RF_GATEWAY_ID     4
#define ADDR_RF_FREQUENCY      5
#define ADDR_ALERT_MULTIPLIER  6
#define ADDR_LOOP_MULTIPLIER   7

bool Mote::_intr = false;
unsigned long Mote::_lastIntrTime = 0;


//-----------------------------------------------------------------------------
// API Methods
//-----------------------------------------------------------------------------
Mote::Mote(const char* name, const char* version, bool init) {
  if (init) {
    initConfig();
    storeConfig();
  } 
  loadConfig();
}

void Mote::setup() {
  Serial.print("setup ports...");
  setupPorts();
  Serial.println("ok!");
  
  Serial.print("setup status indicators...");
  setupStatusIndicator();
  Serial.println("ok!");
  
  Serial.print("setup radio...");
  setupRadio();
  Serial.println("ok!");
}

void Mote::loop() {
  measure();
  report();
  sleep();
}

//-----------------------------------------------------------------------------
// Support Methods
//-----------------------------------------------------------------------------
void Mote::blinkStatusLeds() {
  digitalWrite(_statusLedPin1, HIGH);
  digitalWrite(_statusLedPin2, HIGH);
  delay(calculateLedDelay());  
  digitalWrite(_statusLedPin1, LOW);
  digitalWrite(_statusLedPin2, LOW);
}

void Mote::loadConfig() {
  _statusLedPin1 = EEPROM.read(ADDR_STATUS_LED_1_PIN);
  _statusLedPin2 = EEPROM.read(ADDR_STATUS_LED_2_PIN);
  _rfNodeId = EEPROM.read(ADDR_RF_NODE_ID);
  _rfNetworkId = EEPROM.read(ADDR_RF_NETWORK_ID);
  _rfGatewayId = EEPROM.read(ADDR_RF_GATEWAY_ID);
  _rfFrequency = EEPROM.read(ADDR_RF_FREQUENCY);
  _alertMultiplier = EEPROM.read(ADDR_ALERT_MULTIPLIER);
  _loopMultiplier = EEPROM.read(ADDR_LOOP_MULTIPLIER);
}

void Mote::report() {
  memset(&_outbound, 0, sizeof(_outbound));
  _outbound.msg.type = calculateMessageLevel();
  _outbound.msg.source = _rfNodeId;
  _outbound.msg.destination = 0;
  _outbound.msg.component = 0;
  _outbound.msg.rssi = 0;
  memcpy(&_outbound.msg.data, sensorData(), MSG_DATA_LENGTH);
  
  Serial.print("Broadcasting report to gateway...");
  if (_radio.sendWithRetry(_rfGatewayId, _outbound.raw, MSG_LENGTH)) {
    Serial.println("ACK");
  } else {
    Serial.println("No ACK!");
  }
  
  blinkStatusLeds();
}

void Mote::setupRadio() {
  _radio.initialize(_rfFrequency, _rfNodeId, _rfNetworkId);
  _radio.setHighPower();
  delay(1000);
}

void Mote::setupStatusIndicator() {
  pinMode(_statusLedPin1, OUTPUT);
  pinMode(_statusLedPin2, OUTPUT);
  digitalWrite(_statusLedPin1, LOW);
  digitalWrite(_statusLedPin2, LOW);
}

void Mote::sleep() {
  attachInterrupt(1, wakeup, CHANGE);
  byte multiplier = calculateSleepMultiplier();
  for (int i = 0; i < multiplier; i++) {
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
    if (_intr) {
      _intr = false;
      break;
    }
  }
  detachInterrupt(1);
}

void Mote::storeConfig() {
  EEPROM.write(ADDR_STATUS_LED_1_PIN, _statusLedPin1);
  EEPROM.write(ADDR_STATUS_LED_2_PIN, _statusLedPin2);
  EEPROM.write(ADDR_RF_NODE_ID, _rfNodeId);
  EEPROM.write(ADDR_RF_NETWORK_ID, _rfNetworkId);
  EEPROM.write(ADDR_RF_GATEWAY_ID, _rfGatewayId);
  EEPROM.write(ADDR_RF_FREQUENCY, _rfFrequency);
  EEPROM.write(ADDR_ALERT_MULTIPLIER, _alertMultiplier);
  EEPROM.write(ADDR_LOOP_MULTIPLIER, _loopMultiplier);
}

void Mote::wakeup() {
  _intr = true;
  _lastIntrTime = millis();
}

#endif
