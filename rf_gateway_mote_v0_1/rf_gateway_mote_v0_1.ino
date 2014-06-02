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

#define VERSION "v0.1"

#define SERIAL      1   // set to 1 to also report readings on the serial port
#define DEBUG       1   // set to 1 to display each loop() run
#define BAUD_RATE   57600

#define NODEID      1   // unique for each node on same network
#define GATEWAYID   1
#define NETWORKID   99  // same for all nodes that talk to each other
#define FREQUENCY   RF69_915MHZ

#define RAW_LENGTH    16
#define DATA_LENGTH   RAW_LENGTH - 3

struct MessageData {
    union {
        byte raw[RAW_LENGTH];
        struct {
            byte msgtype;
            byte network;
            byte node;
            byte data[DATA_LENGTH];
        } msg;
    };
};
MessageData message;

char cmd[2] = { '\0', '\0' };

RFM69 radio;


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
  
    if (radio.receiveDone()) {
        if (radio.DATALEN != sizeof(MessageData)) {
            Serial.print("Invalid payload received, not matching Payload struct!");
        } else {
            memcpy(&message, (byte*) radio.DATA, sizeof message);
            consumeInbound();
        }
    }
    
    if (radio.ACK_REQUESTED) {
        radio.sendACK();
    }
    
}

//------------------------------------------------------------------------------
// consume inbound message
//
static void consumeInbound() {
    EmBencode encoder;
    encoder.startList();
    cmd[0] = message.msg.msgtype;
    encoder.push(cmd);
    encoder.push(message.msg.network);
    encoder.push(message.msg.node);
    encoder.startList();
    for (int i = 0; i < DATA_LENGTH; i++) { 
        encoder.push(message.msg.data[i]);
    }
    encoder.endList();
    encoder.endList();
    Serial.println();
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
