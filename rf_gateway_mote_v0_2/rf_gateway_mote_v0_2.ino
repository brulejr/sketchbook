/* -----------------------------------------------------------------------------
   Mote RF Gateway
  
   Monitors several sensors on a freezer watching for out-of-ordinary behavior.
 
   Circuit:
   * GPIO connect to Raspberry Pi
   * RF Status LED attached to digital pin 9
 
   Created 10-JUN-2014 by Jon Brule
----------------------------------------------------------------------------- */
#include <ChibiOS_AVR.h>
#include <RFM69.h>
#include <SPI.h>
#include <EmBencode.h>
#include <avr/sleep.h>
#include <Heartbeat.h>
#include <Event.h>

#define VERSION "v0.2"

#define SERIAL          1   // set to 1 to also report readings on the serial port
#define DEBUG           1   // set to 1 to display each loop() run
#define BAUD_RATE       57600

#define NODEID          1   // unique for each node on same network
#define GATEWAYID       1
#define NETWORKID       99  // same for all nodes that talk to each other
#define FREQUENCY       RF69_915MHZ

#define DPIN_MOTE_LED   9  // moteinos have LEDs on D9

#define SERIAL_CHECK_DELAY 100

RFM69 radio;

EventMessage inbound, outbound;

char embuf[EVENT_LENGTH * 8];
EmBdecode decoder(embuf, sizeof embuf);


//----------------------------------------------------------------------------- 
// heartbeat thread
//
static WORKING_AREA(waThread1, 50);
static msg_t HeartbeatThread(void *arg) {
  Heartbeat heartbeat(DPIN_MOTE_LED);
  while (1) {
    heartbeat.pulse();
  }
  return 0;
}

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {
  
    Serial.begin(BAUD_RATE);
    #if DEBUG
        Serial.print("\n[GatewayMote - ");
        Serial.print(VERSION);
        Serial.println("]");
    #endif
    
    setup_radio();
    
    chBegin(mainThread);
    
}

//-----------------------------------------------------------------------------
void mainThread () {
  
  // start heartbeat thread
  chThdCreateStatic(waThread1, sizeof(waThread1),
                    NORMALPRIO + 1, HeartbeatThread, NULL);

  while (true)
    loop();
}

//------------------------------------------------------------------------------
// main processing loop
//
void loop() {
  
    // bridge rf to serial messages
    noInterrupts();
    if (radio.receiveDone()) {
        if (radio.DATALEN != sizeof(EventMessage)) {
            Serial.print("Invalid payload received, not matching Payload struct!");
        } else {
            memcpy(&inbound, (byte*) radio.DATA, sizeof inbound);
            consumeRf();
        }
        if (radio.ACK_REQUESTED) {
            radio.sendACK();
        }
    }
    interrupts();
    
    // bridge serial messages to rf
    noInterrupts();
    if (consumeSerial()) {
        radio.send(outbound.event.destination, outbound.raw, EVENT_LENGTH);
    }        
    interrupts();
    
}

//------------------------------------------------------------------------------
// consume inbound RF message
//
static void consumeRf() {
    EmBencode encoder;
    encoder.startList();
    encoder.push(inbound.event.type);
    encoder.push(inbound.event.network);
    encoder.push(inbound.event.source);
    encoder.push(inbound.event.destination);
    encoder.startList();
    for (int i = 0; i < EVENT_DATA_LENGTH; i++) { 
        encoder.push(inbound.event.data[i]);
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
          while (i < EVENT_LENGTH) {
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
          if (i < EVENT_LENGTH) {
              for (int j = i; j < EVENT_LENGTH; j++) {
                  outbound.raw[j] = 0x00;
              }
          }
      }
  }
  return pendingOutbound;
}

//------------------------------------------------------------------------------
// Initializes the RF radio.
//
static void setup_radio() {
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

//------------------------------------------------------------------------------
void EmBencode::PushChar (char ch) {
  Serial.write(ch);
}
