/* -----------------------------------------------------------------------------
   Mote RF Gateway
  
   Monitors several sensors on a freezer watching for out-of-ordinary behavior.
 
   Circuit:
   * GPIO connect to Raspberry Pi
   * RF Status LED attached to digital pin 9
 
   Created 01-JUN-2014 by Jon Brule
----------------------------------------------------------------------------- */

#include <JeeLib.h>
#include <RFM69.h>
#include <SPI.h>
#include <EmBencode.h>
#include <avr/sleep.h>
#include "message.h"

#define VERSION "v0.1"

#define SERIAL      1   // set to 1 to also report readings on the serial port
#define DEBUG       1   // set to 1 to display each loop() run
#define BAUD_RATE   57600

#define NODEID      1   // unique for each node on same network
#define GATEWAYID   1
#define NETWORKID   99  // same for all nodes that talk to each other
#define FREQUENCY   RF69_915MHZ

#define DPIN_MOTE_LED   9  // moteinos have LEDs on D9

#define SERIAL_CHECK_DELAY 100

RFM69 radio;

MessageData inbound, outbound;

char embuf[MSG_LENGTH * 8];
EmBdecode decoder(embuf, sizeof embuf);


//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {
  
      #if SERIAL || DEBUG
        Serial.begin(BAUD_RATE);
        Serial.print("\n[GatewayMote - ");
        Serial.print(VERSION);
        Serial.println("]");
        serialFlush();
    #endif
    
    setup_radio();
    
}

//------------------------------------------------------------------------------
// main processing loop
//
void loop() {
  
    heartbeat(DPIN_MOTE_LED);
  
    // bridge rf to serial messages
    if (radio.receiveDone()) {
        if (radio.DATALEN != sizeof(MessageData)) {
            Serial.print("Invalid payload received, not matching Payload struct!");
        } else {
            memcpy(&inbound, (byte*) radio.DATA, sizeof inbound);
            consumeRf();
        }
        if (radio.ACK_REQUESTED) {
            radio.sendACK();
        }
    }
    
    // bridge serial messages to rf
    if (consumeSerial()) {
        radio.sendWithRetry(outbound.msg.node, outbound.raw, MSG_LENGTH);
    }        
    
}

//------------------------------------------------------------------------------
// consume inbound RF message
//
static void consumeRf() {
    EmBencode encoder;
    encoder.startList();
    encoder.push(inbound.msg.msgtype);
    encoder.push(inbound.msg.network);
    encoder.push(inbound.msg.node);
    encoder.startList();
    for (int i = 0; i < MSG_BODY_LENGTH; i++) { 
        encoder.push(inbound.msg.data[i]);
    }
    encoder.endList();
    encoder.endList();
    Serial.println();
}

//------------------------------------------------------------------------------
// generate outbound RF data
//
static boolean consumeSerial() {
  boolean pendingOutbound = false;
  if (Serial.available() > 0) {
      pendingOutbound = true;
      char ch = Serial.read();
      uint8_t bytes = decoder.process(ch);
      if (bytes > 0) {
          uint8_t i = 0;
          while (i < MSG_LENGTH) {
              uint8_t token = decoder.nextToken();
              if (token == EmBdecode::T_END) {
                  break;
              }
              switch (token) {
                  case EmBdecode::T_NUMBER:
                      outbound.raw[i++] = decoder.asNumber();
                      break;
                  case EmBdecode::T_LIST:
                      break;
              }
          }
          decoder.reset();
          if (i < MSG_LENGTH) {
              for (int j = i; j < MSG_LENGTH; j++) {
                  outbound.raw[j] = 0x00;
              }
          }
      }
  }
  return pendingOutbound;
}

//------------------------------------------------------------------------------
// simulates a heartbeat to show activity
//
static void heartbeat(int pin) {
    int rate = 25;
    int pmw = 255;
    for(int i = 0; i < pmw; i++) {
        analogWrite(pin,i);
        delay(((60000/rate)*.1)/pmw);
    }
    for (int i = pmw; i > 0; i--){
        analogWrite(pin,i);
        delay(((60000/rate)*.2)/pmw);
    }
    for(int i = 0; i < pmw; i++) {
        analogWrite(pin,i);
        delay(((60000/rate)*.1)/pmw);
    }
    for (int i = pmw; i > 0; i--){
        analogWrite(pin,i);
        delay(((60000/rate)*.6)/pmw);
    }
}

//------------------------------------------------------------------------------
// flushes the serial port
//
static void serialFlush () {
    #if ARDUINO >= 100
        Serial.flush();
    #endif  
    delay(2); // make sure tx buf is empty before going back to sleep
}

//------------------------------------------------------------------------------
// Initializes the sensors.
//
static void setup_radio() {
    #if DEBUG
        Serial.println("setup radio");
    #endif
    
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
    radio.setHighPower();
}

//------------------------------------------------------------------------------
void EmBencode::PushChar (char ch) {
  Serial.write(ch);
}
