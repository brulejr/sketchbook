#include <LowPower.h>
#include <Reading.h>
#include "Sensors.h"

#define SERIAL     1
#define DEBUG      1
#define BAUD_RATE  9600

#define VERSION    "v0.1"

const int MOTE_LED_PIN = 9;
const int EXTR_LED_PIN = 13;

Sensors* sensors;
SensorData* sensorData;


//-----------------------------------------------------------------------------
// Initialization
//
void setup() {
  #if SERIAL || DEBUG
    Serial.begin(BAUD_RATE);
    Serial.print("\n[laundrymote - ");
    Serial.print(VERSION);
    Serial.println("]");
    delay(100);
  #endif
  
  setupPorts();
  setupLeds();
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

void setupSensors() {
  sensors = new Sensors();
  sensorData = new SensorData();
  memset(sensorData, 0, sizeof(*sensorData));
  attachInterrupt(1, waterLeakChange, CHANGE);
}


//-----------------------------------------------------------------------------
// Main Loop
//
void loop() {
  enableISRs();
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
  disableISRs();
  
  sensors->measure();
  sensors->report(sensorData);
  
  blink(MOTE_LED_PIN, EXTR_LED_PIN, sensorData);
}


//-----------------------------------------------------------------------------
// Support routines
//
void enableISRs() {
  attachInterrupt(1, waterLeakChange, CHANGE);
}

void disableISRs() {
  detachInterrupt(1);
}

void blink(const int pin1, const int pin2, SensorData* sensorData) {
  digitalWrite(pin1, HIGH);
  digitalWrite(pin2, HIGH);
  delay((sensorData->waterLeak) ? 500 : 100);
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
}

void waterLeakChange() {
  #if DEBUG
    Serial.println("WATER LEAK Change");
  #endif
}

