#include <LowPower.h>
#include <RFM69.h>
#include <SPI.h>

#define LED 9

RFM69 radio;

void setup() {
  radio.initialize(RF69_915MHZ, 99, 99);
  radio.setHighPower();
  pinMode(LED, OUTPUT);
  PORTD = B00000000;  // enable pullups on pins 2 to 7, leave pins 0 and 1 alone
  PORTB = B00000000;  // enable pullups on pins 2 to 7, leave pins 0 and 1 alone
}

void loop() {
  digitalWrite(LED, HIGH);
  delay(4000);
  digitalWrite(LED, LOW);
  radio.sleep();
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
}
