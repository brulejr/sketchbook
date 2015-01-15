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

#include <Message.h>
#include <ArduinoJson.h>


#define VERSION "v0.1"

#define SERIAL          1   // set to 1 to also report readings on the serial port
#define DEBUG           1   // set to 1 to display each loop() run
#define BAUD_RATE       57600

#define MQTT_PORT       1883

#define DPIN_MOTE_LED    9  // heartbeat
#define DPIN_MQTT_LED   13  // mqtt status

#define I2C_BUFFER_SIZE MSG_DATA_LENGTH * 10
#define ETH_BUFFER_SIZE MSG_DATA_LENGTH * 10


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
  
  #if DEBUG
    Serial.print("I2C<direction=");
    Serial.print(msgI2C.msg.direction, DEC);
    Serial.print(",type=");
    Serial.print(msgI2C.msg.type, DEC);
    Serial.print(",source=");
    Serial.print(msgI2C.msg.source, DEC);
    Serial.print(",destination=");
    Serial.print(msgI2C.msg.destination, DEC);
    Serial.print(",rssi=");
    Serial.print(msgI2C.msg.rssi, DEC);
    Serial.print(",rssi=data(");
    for (int i = 0; i < MSG_DATA_LENGTH; i++) {
      if (i > 0) Serial.print(",");
      Serial.print(msgI2C.msg.data[i], DEC);
    }
    Serial.println(")>");
  #endif

  haveDataI2C = true; 
}

//------------------------------------------------------------------------------
// Publishes an I2C message to a MQTT topic.
//
void sendToMQTT() {
  
  memset(i2cbuf, 0, sizeof(i2cbuf));
    
  // format message as json
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["direction"] = msgI2C.msg.direction;
  root["type"] = msgI2C.msg.type;
  root["source"] = msgI2C.msg.source;
  root["destination"] = msgI2C.msg.destination;
  root["rssi"] = msgI2C.msg.rssi;
  JsonArray& data = root.createNestedArray("data");
  for (int i = 0; i < MSG_DATA_LENGTH; i++) {  
    data.add(msgI2C.msg.data[i]);
  }
  root.printTo(i2cbuf, sizeof(i2cbuf));
  
  #if DEBUG
    Serial.print("i2c -> mqtt: [");
    root.printTo(Serial);
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

