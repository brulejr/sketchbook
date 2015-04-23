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
    byte _alertMultiplier = 2;
    byte _loopMultiplier = 10;
    byte _rfNodeId     = 9;
    byte _rfNetworkId  = 99;
    byte _rfGatewayId  = 1;
    byte _rfFrequency  = RF69_915MHZ;
    byte _statusLedPin1 = 9;
    byte _statusLedPin2 = 7;
    
    static void wakeup();
    static bool _alert;
};

#define ADDR_STATUS_LED_1_PIN  0
#define ADDR_STATUS_LED_2_PIN  1
#define ADDR_RF_NODE_ID        2
#define ADDR_RF_NETWORK_ID     3
#define ADDR_RF_GATEWAY_ID     4
#define ADDR_RF_FREQUENCY      5
#define ADDR_ALERT_MULTIPLIER  6
#define ADDR_LOOP_MULTIPLIER   7

bool Mote::_alert = false;

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

void Mote::blinkStatusLeds() {
  digitalWrite(_statusLedPin1, HIGH);
  digitalWrite(_statusLedPin2, HIGH);
  delay((_alert) ? 500 : 100);  
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
  _outbound.msg.type = (_alert) ? MSG_ALERT : MSG_READING;
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

void Mote::setupPorts() {
  Serial.print("setup ports...");
  DDRD  = B10000011;  // set Arduino pins 2 to 7 as inputs, leaves 0 & 1 (RX & TX) as is
  DDRB  = B00000000;  // set pins 8 to 13 as inputs
  PORTD = B01110100;  // enable pullups on pins 2 to 7, leave pins 0 and 1 alone
  PORTB = B11111111;  // enable pullups on pins 8 to 13
  Serial.println("ok!");
}

void Mote::setupRadio() {
  Serial.print("setup radio...");
  _radio.initialize(_rfFrequency, _rfNodeId, _rfNetworkId);
  _radio.setHighPower();
  delay(1000);
  Serial.println("ok!");
}

void Mote::setupStatusIndicator() {
  Serial.print("setup status indicators...");
  pinMode(_statusLedPin1, OUTPUT);
  pinMode(_statusLedPin2, OUTPUT);
  digitalWrite(_statusLedPin1, LOW);
  digitalWrite(_statusLedPin2, LOW);
  Serial.println("ok!");
}

void Mote::sleep() {
  attachInterrupt(1, wakeup, CHANGE);
  byte multiplier = (_alert) ? _alertMultiplier : _loopMultiplier;
  for (int i = 0; i < multiplier; i++) {
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
    if (_alert) break;
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
  _alert = true;
}

#endif
