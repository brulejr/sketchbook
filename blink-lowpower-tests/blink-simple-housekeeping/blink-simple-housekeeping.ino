/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

// the setup function runs once when you press reset or power the board
void setup() {
  DDRD &= B00000011;    // set Arduino pins 2 to 7 as inputs, leaves 0 & 1 (RX & TX) as is
  DDRB = B00000000;      // set pins 8 to 13 as inputs
  PORTD |= B11111100;    // enable pullups on pins 2 to 7, leave pins 0 and 1 alone
  PORTB |= B11111111;    // enable pullups on pins 8 to 13
  pinMode(13, OUTPUT);   // set pin 13 as an output so we can use LED to monitor
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(3000);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(3000);              // wait for a second
}
