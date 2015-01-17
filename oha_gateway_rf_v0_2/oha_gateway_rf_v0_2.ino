/* -----------------------------------------------------------------------------
   OpenHAB RF Gateway
  
   Transfers messages between an RF Mesh and the Serial port ina bi-directional
   manner. Messages on the RF Mesh use the Message structure, whereas messages 
   on the Serial port are in JSON form.
 
   Circuit:
   * FTDI port connects to host for Serial communcations and programming
   * Heartbeat LED attached to digital pin 9
   * RF Status LED attached to digital pin 12
   * Serial Status LED attached to digital pin 13
 
   Created 76-JAN-2015 by Jon Brule
----------------------------------------------------------------------------- */
#include <SPI.h>
#include <RFM69.h>
#include <Message.h>
#include <ArduinoJson.h>


#define VERSION "v0.2"

#define SERIAL          1   // set to 1 to also report readings on the serial port
#define DEBUG           1   // set to 1 to display each loop() run
#define BAUD_RATE       57600

#define NODEID          1   // unique for each node on same network
#define NETWORKID       99  // same for all nodes that talk to each other
#define FREQUENCY       RF69_915MHZ

#define DPIN_MOTE_LED   9   // heartbeat
#define DPIN_RFM_LED    12  // rf message indicator
#define DPIN_SERM_LED   13  // serial message indicator

#define MAX_BUFFER_SIZE MSG_DATA_LENGTH * 10


// RF configuration
RFM69 radio;

// Messages & buffers
Message rfMsg, serialMsg;
char serialBuffer[MAX_BUFFER_SIZE];


//---------------------------------------------------------------------------// 
// SETUP
//---------------------------------------------------------------------------// 

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {  
  setup_serial();
  setup_rf();
  send_ident_msg();
}

//------------------------------------------------------------------------------
// Initializes the RF radio communications.
//
static void setup_rf() {
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  radio.setHighPower();
  delay(1000);
}

//------------------------------------------------------------------------------
// Initializes the Serial communications.
//
static void setup_serial() {
  Serial.begin(BAUD_RATE);
}

static void send_ident_msg() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = MSG_BOOTSTRAP;
  root["id"] = "OHA RF Gateway";
  root["version"] = VERSION;
  JsonObject& rf = root.createNestedObject("rf");
  rf["nodeID"] = NODEID;
  rf["networkID"] = NETWORKID;
  rf["frequency"] = FREQUENCY;
  root.printTo(Serial);
  Serial.println();
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
    sendToSerial();
  }
    
  // process inbound Serial message
  if (receiveFromSerial()) {
    sendToRF();
  }

}

//------------------------------------------------------------------------------
// Receives a message from the RF mesh
//
boolean receiveFromRF() {
  boolean haveData = false;
  if (radio.receiveDone()) {
    if (radio.DATALEN != sizeof(Message)) {
      Serial.print("Invalid payload received, not matching Message struct!");
    } else {
      memcpy(&rfMsg, (byte*) radio.DATA, sizeof rfMsg);
      rfMsg.msg.rssi = radio.RSSI;
      haveData = true;
    }
    if (radio.ACK_REQUESTED) {
      radio.sendACK();
    }
  }
  return haveData;
}

//------------------------------------------------------------------------------
// Receives a message from the Serial port.
//
boolean receiveFromSerial() {
  boolean haveData = false;
  if (readline(Serial.read(), serialBuffer, MAX_BUFFER_SIZE) > 0) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(serialBuffer);
    serialMsg.msg.type = root["type"];
    serialMsg.msg.direction = root["direction"];
    serialMsg.msg.source = root["source"];
    serialMsg.msg.destination = root["destination"];
    serialMsg.msg.rssi = 0x00;
    for (int i = 0; i < MSG_DATA_LENGTH; i++) {
      serialMsg.msg.data[i] = root["data"][i];
    }
    haveData = true;
  }
  return haveData;
}

//------------------------------------------------------------------------------
// Publishes an I2C message to the RF mesh.
//
void sendToRF() {
  radio.send(serialMsg.msg.destination, serialMsg.raw, MSG_LENGTH);
}

//------------------------------------------------------------------------------
// Publishes an RF message to the Serial port.
//
void sendToSerial() {
    
  // format message as json
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = rfMsg.msg.type;
  root["direction"] = rfMsg.msg.direction;
  root["source"] = rfMsg.msg.source;
  root["destination"] = rfMsg.msg.destination;
  root["rssi"] = rfMsg.msg.rssi;
  JsonArray& data = root.createNestedArray("data");
  for (int i = 0; i < MSG_DATA_LENGTH; i++) {  
    data.add(rfMsg.msg.data[i]);
  }

  // generate json string to serial port
  root.printTo(Serial);
  Serial.println();
  
}


//---------------------------------------------------------------------------// 
// SUPPORT METHODS
//---------------------------------------------------------------------------// 

//------------------------------------------------------------------------------
// Reads a line from the Serial port.
//
int readline(int readch, char *buffer, int len) {
  static int pos = 0;
  int rpos;

  if (readch > 0) {
    switch (readch) {
      case '\n': // Ignore new-lines
        break;
      case '\r': // Return on CR
        rpos = pos;
        pos = 0;  // Reset position index ready for next time
        return rpos;
      default:
        if (pos < len-1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
    }
  }
  
  // No end of line has been found, so return -1.
  return -1;
}

