/* -----------------------------------------------------------------------------
   OpenHAB Ethernet Gateway
  
   Transfers messages between an Ethernet MQTT and the I2C bus. Messages on the
   I2C bus use the Event structure form, whereas those sent and received by MQTT
   are bencoded (http://en.wikipedia.org/wiki/Bencode).
 
   Circuit:
   * GPIO connect to Raspberry Pi
   * RF Status LED attached to digital pin 9
 
   Created 06-JAN-2015 by Jon Brule
----------------------------------------------------------------------------- */
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Wire.h>

#include <EmBencode.h>
#include <Message.h>


#define VERSION "v0.1"

#define SERIAL          1   // set to 1 to also report readings on the serial port
#define DEBUG           1   // set to 1 to display each loop() run
#define BAUD_RATE       57600

#define MQTT_PORT       1883

#define DPIN_MOTE_LED    9  // heartbeat
#define DPIN_MQTT_LED   13  // mqtt status

#define I2C_BUFFER_SIZE MSG_DATA_LENGTH * 8
#define ETH_BUFFER_SIZE MSG_DATA_LENGTH * 8


// Ethernet configuration
byte mac[]    = {  0x90, 0xA2, 0xDA, 0x0D, 0x11, 0x11 };
byte ip[]     = { 192, 168, 100, 66 };
byte server[] = { 192, 168, 100, 88 };
EthernetClient ethClient;

// I2C receive device address
const byte I2C_ADDR = 42;

// MQTT configuration
PubSubClient client(server, MQTT_PORT, receiveFromMQTT, ethClient);
char clientName[] = "oha:gateway";
char topicName[] = "oha/rf/msg";
unsigned long keepalivetime = 0;
unsigned long MQTT_reconnect = 0;

int sendMQTT = 0;
volatile boolean haveDataI2C = false;
volatile boolean haveDataMQTT = false;

// Message 
Message msgI2C, msgEthernet;
char i2cbuf[I2C_BUFFER_SIZE];
char ethbuf[ETH_BUFFER_SIZE];
//EmBdecode decoder(embuf, sizeof embuf);

byte *i2cbp, *ethbp;


//---------------------------------------------------------------------------// 
// SETUP
//---------------------------------------------------------------------------// 

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {  
  Serial.begin(BAUD_RATE);
  #if SERIAL
    Serial.print("\n[OHA Gateway - ");
    Serial.print(VERSION);
    Serial.println("]");
  #endif
    
  setup_ethernet();
  setup_i2c();
  setup_mqtt();    
}

//------------------------------------------------------------------------------
// Initializes the Ethernet using DHCP.
//
static void setup_ethernet() {
  #if SERIAL
    Serial.print("Setup ethernet...");
  #endif
  while (Ethernet.begin(mac) != 1) {
    delay(3000);
    Serial.print(".");
  }
  delay(1000);
  #if SERIAL
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
  #if SERIAL
    Serial.println("OK");
  #endif
}

//------------------------------------------------------------------------------
// Initializes the MQTT service.
//
static void setup_mqtt() {
  #if SERIAL
    Serial.print("Setup MQTT..");
  #endif
  while (!client.connected()) {
    Serial.print(".");
    client.connect(clientName);
    delay(1000);
  }
  MQTT_reconnect = millis();
  #if SERIAL
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
  
  // process inbound I2C message
  if (haveDataI2C) {
    sendToMQTT();
    haveDataI2C = false;
  }
    
  // process inbound MQTT message
  if (haveDataMQTT) {
    sendToI2C();
    haveDataMQTT = false;
  }
  
  client.loop();
}

//------------------------------------------------------------------------------
// Receives a message from the Etherenet (a.k.a. MQTT)
//
void receiveFromMQTT(char* topic, byte* payload, unsigned int length) {
  // handle MQTTmessage arrived
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
// Publishes an I2C message to a MQTT topic.
//
void sendToMQTT() {
  
  memset(i2cbuf, 0, sizeof(i2cbuf));
  i2cbp = (byte *) &i2cbuf;
    
  // format message as bencode
  EmBencode encoder;
  encoder.startList();
  encoder.push(msgI2C.msg.direction);
  encoder.push(msgI2C.msg.type);
  encoder.push(msgI2C.msg.source);
  encoder.push(msgI2C.msg.destination);
  encoder.push(msgI2C.msg.rssi);
  encoder.startList();
  for (int i = 0; i < MSG_DATA_LENGTH; i++) { 
    encoder.push(msgI2C.msg.data[i]);
  }
  encoder.endList();
  encoder.endList();
  
  #if DEBUG
    Serial.print("i2c -> mqtt: [");
    Serial.print(i2cbuf);
    Serial.println("]");
  #endif
    
  // send message to MQTT topic
  client.publish(topicName, i2cbuf);  
}

//------------------------------------------------------------------------------
// Publishes an MQTT message to the I2C buss.
//
void sendToI2C() {
}

//------------------------------------------------------------------------------
void EmBencode::PushChar(char ch) {
  *i2cbp++ = ch;
}

