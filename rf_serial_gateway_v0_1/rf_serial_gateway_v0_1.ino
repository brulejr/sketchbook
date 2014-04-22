/// @dir rf12serial
/// Serial data over RF12 demo, works in both directions.
// 2009-04-17 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <JeeLib.h>
#include <EmBencode.h>

#define VERSION "v0.1"

#define DEBUG      1     // set to 1 to enable defug mode
#define BAUD_RATE  57600

#define NODEID     1
#define NETWORKID  99
#define FREQUENCY  RF12_915MHZ

#define RAW_LENGTH 16

typedef struct {
    byte type;
    byte format;
    byte data[RAW_LENGTH];
} Payload;

MilliTimer sendTimer;
Payload inData, outData;
byte pendingOutput;

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {
    Serial.begin(BAUD_RATE);
    #if DEBUG
        Serial.print("[rfSerialGateway - ");
        Serial.print(VERSION);
        Serial.print("]");
    #endif    
    rf12_initialize(NODEID, FREQUENCY, NETWORKID);
}

//------------------------------------------------------------------------------
// main processing loop
//
void loop() {
  
    if (rf12_recvDone() && rf12_crc == 0 && rf12_len == sizeof inData) {
        memcpy(&inData, (byte*) rf12_data, sizeof inData);
        // optional: rf12_recvDone(); // re-enable reception right away
        consumeInData();
    }

    if (sendTimer.poll(100))
        pendingOutput = produceOutData();

    if (pendingOutput && rf12_canSend()) {
        rf12_sendNow(0, &outData, sizeof outData);
        // optional: rf12_sendWait(2); // wait for send to finish
        pendingOutput = 0;
    }  
    
}

//------------------------------------------------------------------------------
// consume inbound data
//
static void consumeInData () {
    EmBencode encoder;
    encoder.push(inData.type);
    encoder.push(inData.format);
    encoder.startList();
    for (int i = 0; i < RAW_LENGTH; i++) { 
        encoder.push(inData.data[i]);
    }
    encoder.endList();
}

//------------------------------------------------------------------------------
// generate outbound data
//
static byte produceOutData () {
  if (Serial.available()) {
      return 1;
  } else {
      return 0;
  }
}


//------------------------------------------------------------------------------
void EmBencode::PushChar (char ch) {
  Serial.print(ch);
}
