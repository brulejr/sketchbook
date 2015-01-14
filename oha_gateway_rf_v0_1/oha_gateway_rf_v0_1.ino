/* -----------------------------------------------------------------------------
   OpenHAB RF Gateway
  
   Transfers messages between an RF Mesh and the I2C bus. Messages on both the
   I2C bus and the RF Mesh use the Message structure.
 
   Circuit:
   * Analog 4 to Arduino 4 via level shifter
   * Analog 5 to Arduino 5 via level shifter
   * RF Status LED attached to digital pin 9
 
   Created 13-JAN-2015 by Jon Brule
----------------------------------------------------------------------------- */
#include <RFM69.h>
#include <SPI.h>
#include <Wire.h>
#include <Message.h>


#define VERSION "v0.1"

#define SERIAL          1   // set to 1 to also report readings on the serial port
#define DEBUG           1   // set to 1 to display each loop() run
#define BAUD_RATE       57600

#define NODEID          1   // unique for each node on same network
#define GATEWAYID       1
#define NETWORKID       99  // same for all nodes that talk to each other
#define FREQUENCY       RF69_915MHZ

#define DPIN_MOTE_LED    9  // heartbeat
#define DPIN_MQTT_LED   13  // mqtt status


// I2C receive device address
const byte I2C_ADDR = 21;
const byte I2C_TARGET = 42;

// RF configuration
RFM69 radio;

volatile boolean haveDataI2C = false;
volatile boolean haveDataRF = false;

// Message 
Message msgI2C, msgRF;


//---------------------------------------------------------------------------// 
// SETUP
//---------------------------------------------------------------------------// 

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {  
  Serial.begin(BAUD_RATE);
  #if SERIAL
    Serial.print("\n[OHA RF Gateway - ");
    Serial.print(VERSION);
    Serial.println("]");
  #endif
    
  setup_rf();
  setup_i2c();
}

//------------------------------------------------------------------------------
// Initializes the RF radio.
//
static void setup_rf() {
  #if SERIAL
    Serial.print("Setup radio...");
  #endif
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  radio.setHighPower();
  delay(1000);
  #if DEBUG
    Serial.println("OK");
  #endif
}

//------------------------------------------------------------------------------
// Initializes the I2C bus.
//
static void setup_i2c() {
  #if SERIAL
    Serial.print("Setup I2C...");
  #endif
  Wire.begin (I2C_ADDR);
  Wire.onReceive(receiveFromWire);
  delay(1000);
  #if DEBUG
    Serial.println("OK");
  #endif
}


//---------------------------------------------------------------------------// 
// MAIN PROCESSING
//---------------------------------------------------------------------------// 

//------------------------------------------------------------------------------
// main processing loop
//
void loop() {
    
  // process inbound RF message
  if (receiveFromRF()) {
    sendToI2C();
    haveDataRF = false;
  }
    
  // process inbound I2C message
  if (haveDataI2C) {
    sendToRF();
    haveDataI2C = false;
  }

}

//------------------------------------------------------------------------------
// Receives a message from the RF mesh
//
boolean receiveFromRF() {
  if (radio.receiveDone()) {
    if (radio.DATALEN != sizeof(Message)) {
      Serial.print("Invalid payload received, not matching Message struct!");
    } else {
      memcpy(&msgRF, (byte*) radio.DATA, sizeof msgRF);
      msgRF.msg.rssi = radio.RSSI;
      haveDataRF = true;
      #if DEBUG
        Serial.print("RF<direction=");
        Serial.print(msgRF.msg.direction, DEC);
        Serial.print(",type=");
        Serial.print(msgRF.msg.type, DEC);
        Serial.print(",source=");
        Serial.print(msgRF.msg.source, DEC);
        Serial.print(",destination=");
        Serial.print(msgRF.msg.destination, DEC);
        Serial.print(",rssi=");
        Serial.print(msgRF.msg.rssi, DEC);
        Serial.print(",rssi=data(");
        for (int i = 0; i < MSG_DATA_LENGTH; i++) {
          if (i > 0) Serial.print(",");
          Serial.print(msgRF.msg.data[i], DEC);
        }
        Serial.print(")>");
      #endif
    }
    if (radio.ACK_REQUESTED) {
      radio.sendACK();
    }
  }
  return haveDataRF;
}

//------------------------------------------------------------------------------
// Receives a message from the I2C bus (a.k.a Wire).
//
void receiveFromWire(int howMany) {
  if (howMany < sizeof msgI2C) {
    return;
  }

  // read into structure
  byte * p = (byte *) &msgI2C;
  for (byte i = 0; i < sizeof msgI2C; i++) {
    *p++ = Wire.read();
  }
  haveDataI2C = true; 
}

//------------------------------------------------------------------------------
// Publishes an I2C message to the RF mesh.
//
void sendToRF() {
  radio.send(msgI2C.msg.destination, msgI2C.raw, MSG_LENGTH);
}

//------------------------------------------------------------------------------
// Publishes an RF message to the I2C bus.
//
void sendToI2C() {
  #if DEBUG
    Serial.println(" - xmit to i2c");
  #endif
  Wire.beginTransmission(I2C_TARGET);
  Wire.write((byte *) &msgRF, sizeof msgRF);
  Wire.endTransmission();
}

