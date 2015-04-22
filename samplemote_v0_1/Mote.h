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
    inline virtual void initConfig() { /*nothing*/ };
    void loadConfig();
    void report();
    inline virtual byte* sensorData() { return null; };
    void setupPorts();
    void setupRadio();
    void setupStatusIndicator();
    void sleep();
    void storeConfig();
    
    Message _outbound;
    RFM69 _radio;
    byte _rfNodeId     = 9;
    byte _rfNetworkId  = 99;
    byte _rfGatewayId  = 1;
    byte _rfFrequency  = RF69_915MHZ;
    byte _statusLedPin = 9;
};

#define ADDR_STATUS_LED_PIN   0
#define ADDR_RF_NODE_ID       1
#define ADDR_RF_NETWORK_ID    2
#define ADDR_RF_GATEWAY_ID    3
#define ADDR_RF_FREQUENCY     4

Mote::Mote(const char* name, const char* version, bool init) {
  if (init) {
    initConfig();
    storeConfig();
  } 
  loadConfig();
}

void Mote::setup() {
  setupPorts();
  setupStatusIndicator();
  setupRadio();
}

void Mote::loop() {
  measure();
  report();
  sleep();
}

void Mote::loadConfig() {
  _statusLedPin = EEPROM.read(ADDR_STATUS_LED_PIN);
  _rfNodeId = EEPROM.read(ADDR_RF_NODE_ID);
  _rfNetworkId = EEPROM.read(ADDR_RF_NETWORK_ID);
  _rfGatewayId = EEPROM.read(ADDR_RF_GATEWAY_ID);
  _rfFrequency = EEPROM.read(ADDR_RF_FREQUENCY);
}

void Mote::report() {
  digitalWrite(_statusLedPin, HIGH);
  
  memset(&_outbound, 0, sizeof(_outbound));
  _outbound.msg.type = MSG_READING;
  _outbound.msg.source = _rfNodeId;
  _outbound.msg.destination = 0;
  _outbound.msg.component = 0;
  _outbound.msg.rssi = 0;
  memcpy(&_outbound.msg.data, sensorData(), MSG_DATA_LENGTH);
  
  _radio.sendWithRetry(_rfGatewayId, _outbound.raw, MSG_LENGTH);
  
  delay(100);
  
  digitalWrite(_statusLedPin, LOW);
}

void Mote::setupPorts() {
  DDRD &= B00000011;   // set Arduino pins 2 to 7 as inputs, leaves 0 & 1 (RX & TX) as is
  DDRB = B00000000;    // set pins 8 to 13 as inputs
  PORTD |= B11111100;  // enable pullups on pins 2 to 7, leave pins 0 and 1 alone
  PORTB |= B11111111;  // enable pullups on pins 8 to 13
}

void Mote::setupRadio() {
  _radio.initialize(_rfFrequency, _rfNodeId, _rfNetworkId);
  _radio.setHighPower();
  delay(1000);  
}

void Mote::setupStatusIndicator() {
  pinMode(_statusLedPin, OUTPUT);
  digitalWrite(_statusLedPin, LOW);
}

void Mote::sleep() {
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
}

void Mote::storeConfig() {
  EEPROM.write(ADDR_STATUS_LED_PIN, _statusLedPin);
  EEPROM.write(ADDR_RF_NODE_ID, _rfNodeId);
  EEPROM.write(ADDR_RF_NETWORK_ID, _rfNetworkId);
  EEPROM.write(ADDR_RF_GATEWAY_ID, _rfGatewayId);
  EEPROM.write(ADDR_RF_FREQUENCY, _rfFrequency);
}

#endif
