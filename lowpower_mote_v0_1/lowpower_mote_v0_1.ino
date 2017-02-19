#include <LowPower.h>
#include <RFM69.h>
#include <SPI.h>

#define LED 9

RFM69 radio;

void setup() {
  radio.initialize(RF69_915MHZ, 99, 99);
  radio.setHighPower();
  pinMode(LED, OUTPUT);
}

void loop() {
  digitalWrite(LED, HIGH);
  delay(4000);
  digitalWrite(LED, LOW);
  radio.sleep();
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
}
