#include "LowPower.h"

// Configure wake up pin as input.
// This will consumes few uA of current.

const int LED_PIN = 13;
const int WAKEUP_PIN = 3;

void setup() {
  // DDRD &= B00000011;   // set Arduino pins 2 to 7 as inputs, leaves 0 & 1 (RX & TX) as is
  // DDRB = B00000000;    // set pins 8 to 13 as inputs
  // PORTD |= B11111100;  // enable pullups on pins 2 to 7, leave pins 0 and 1 alone
  // PORTB |= B11111111;  // enable pullups on pins 8 to 13
  pinMode(LED_PIN, OUTPUT);  
  pinMode(WAKEUP_PIN, INPUT);   
}

void loop() {

  // Allow wake up pin to trigger interrupt on low.
  //attachInterrupt(0, wakeUp, LOW);
    
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when timer expires or wake up pin is low.
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);      
    
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(0); 
    
  // Do something here
  // Example: Read sensor, data logging, data transmission.
  ///blink(4000);
}

void wakeUp() {
  noInterrupts();
  for (int i = 1; i < 3; i++) {
    blink(500);
  }
  interrupts();
}

void blink(int delayMs) {
  digitalWrite(LED_PIN, HIGH);
  delay(delayMs);
  digitalWrite(LED_PIN, LOW);  
}
