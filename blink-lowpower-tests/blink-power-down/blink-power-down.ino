#include "LowPower.h"

#define SERIAL     1
#define DEBUG      1
#define BAUD_RATE  9600

#define VERSION    "v0.1"

const int LED_PIN = 13;
const int WAKEUP_PIN = 3;
const int MANY = 4;
const int FEW = 1;

volatile int blinkTimes;

void setup() {
  #if SERIAL || DEBUG
    Serial.begin(BAUD_RATE);
    Serial.print("\n[blink-power-down - ");
    Serial.print(VERSION);
    Serial.println("]");
    delay(100);
  #endif
  
  DDRD &= B00000011;   // set Arduino pins 2 to 7 as inputs, leaves 0 & 1 (RX & TX) as is
  DDRB = B00000000;    // set pins 8 to 13 as inputs
  PORTD |= B11111100;  // enable pullups on pins 2 to 7, leave pins 0 and 1 alone
  PORTB |= B11111111;  // enable pullups on pins 8 to 13
  pinMode(LED_PIN, OUTPUT);
  pinMode(WAKEUP_PIN, INPUT);

  digitalWrite(LED_PIN, LOW);
  adjustBlinkTimes(WAKEUP_PIN);
}

void loop() {  
  attachInterrupt(1, wakeUp, CHANGE);
  LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
  detachInterrupt(1); 
  blink(LED_PIN, blinkTimes);
}

void blink(const int pin, const int times) {
  #if DEBUG
    Serial.println("blink() called");
  #endif
  for (int i = 1; i <= times; i++) {
    digitalWrite(pin, HIGH);
    delay(100);
    digitalWrite(pin, LOW);
    delay(100);
  }
}

void adjustBlinkTimes(const int pin) {
  blinkTimes = (digitalRead(pin) == HIGH) ? MANY : FEW;
}

void wakeUp() {
  #if DEBUG
    Serial.println("wakeUp() called");
  #endif 
  adjustBlinkTimes(WAKEUP_PIN);
}

