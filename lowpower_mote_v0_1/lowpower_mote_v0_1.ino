#include <LowPower.h>
#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>

#define LED 9

RFM69 radio;
SPIFlash flash(8, 0xEF30);

void setup() {
  radio.initialize(RF69_915MHZ, 99, 99);
  radio.setHighPower();
  flash.initialize();
  pinMode(LED, OUTPUT);
}

void loop() {
  digitalWrite(LED, HIGH);
  delay(4000);
  digitalWrite(LED, LOW);
  radio.sleep();
  flash.sleep();
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
  flash.wakeup();
}
