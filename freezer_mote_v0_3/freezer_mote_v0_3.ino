/* -----------------------------------------------------------------------------
   Freezer Mote Sensor
  
   Monitors a freezer.
 
   Circuit:
   * Moteino R4 w/ RFM69HW RF module
   * A1  - Light detector
   * A2  - Temperature (inside)
   * A3  - Temperature (outside)
   * D3  - Door detector
   * D7  - External heartbeat LED
   * D9  - Internal heartbeat LED
 
   Created 13-APR-2015 by Jon Brule
----------------------------------------------------------------------------- */
#include <LowPower.h>
#include <Message.h>
#include <RFM69.h>
#include <Reading.h>
#include <SPI.h>
#include "Sensors.h"

#define VERSION    "v0.3"

#define SERIAL     1
#define DEBUG      1
#define BAUD_RATE  9600

#define NODEID         9   // unique for each node on same network
#define GATEWAYID      1
#define NETWORKID      99  // same for all nodes that talk to each other
#define FREQUENCY      RF69_915MHZ

const int EXTR_LED_PIN = 7;
const int MOTE_LED_PIN = 9;

Sensors* sensors;
SensorData* sensorData;

RFM69 radio;
Message inbound, outbound;


//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------

void setup() {
  #if SERIAL || DEBUG
    Serial.begin(BAUD_RATE);
    Serial.print("\n[freezermote - ");
    Serial.print(VERSION);
    Serial.println("]");
    delay(100);
  #endif
  
  setupPorts();
  setupLeds();
  setupRadio();
  setupSensors();
}

void setupPorts() {
  DDRD &= B00000011;   // set Arduino pins 2 to 7 as inputs, leaves 0 & 1 (RX & TX) as is
  DDRB = B00000000;    // set pins 8 to 13 as inputs
  PORTD |= B11111100;  // enable pullups on pins 2 to 7, leave pins 0 and 1 alone
  PORTB |= B11111111;  // enable pullups on pins 8 to 13
}

void setupLeds() {
  pinMode(MOTE_LED_PIN, OUTPUT);
  pinMode(EXTR_LED_PIN, OUTPUT);
  digitalWrite(MOTE_LED_PIN, LOW);
  digitalWrite(EXTR_LED_PIN, LOW);
}

void setupRadio() {
  #if DEBUG
    Serial.print("setup radio...");
  #endif
  
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.setHighPower();
  delay(1000);
  
  #if DEBUG
    Serial.println("ok!");
  #endif
}

void setupSensors() {
  sensors = new Sensors();
  sensorData = new SensorData();
  memset(sensorData, 0, sizeof(*sensorData));
}


//-----------------------------------------------------------------------------
// Main Loop
//-----------------------------------------------------------------------------

void loop() {  
  sensors->measure();
  sensors->report(sensorData);
  sendToRF((sensorData->door) ? MSG_ALERT : MSG_READING, 0);
  
  blink(MOTE_LED_PIN, EXTR_LED_PIN, sensorData);
  
  sleep(sensorData);
}


//-----------------------------------------------------------------------------
// Support routines
//-----------------------------------------------------------------------------

void blink(const int pin1, const int pin2, SensorData* sensorData) {
  digitalWrite(pin1, HIGH);
  digitalWrite(pin2, HIGH);
  delay((sensorData->door) ? 500 : 100);
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
}

void sleep(SensorData* sensorData) {
  attachInterrupt(1, wakeup, CHANGE);
  period_t period = (sensorData->door) ? SLEEP_1S : SLEEP_8S;
  LowPower.powerDown(period, ADC_OFF, BOD_OFF);
  detachInterrupt(1);
}

void sendToRF(byte type, byte component) {
  memset(&outbound, 0, sizeof(outbound));
  outbound.msg.type = type;
  outbound.msg.source = NODEID;
  outbound.msg.destination = 0;
  outbound.msg.component = component;
  outbound.msg.rssi = 0;
  memcpy(&outbound.msg.data, sensorData, sizeof(*sensorData));
    
  #if DEBUG
    Serial.print("Broadcasting report to gateway...");
  #endif
  if (radio.sendWithRetry(GATEWAYID, outbound.raw, MSG_LENGTH)) {
    #if DEBUG
      Serial.println("ACK");
    #endif
  } else {
    #if DEBUG
      Serial.println("No ACK!");
    #endif
  }
}

void wakeup() {
  #if DEBUG
    Serial.println("DOOR Change");
  #endif
}
