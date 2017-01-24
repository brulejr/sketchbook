/* -----------------------------------------------------------------------------
   Ethernet Water Sensor
  
   Monitors water overflow as well as temperature and humidity watching for out-of-ordinary behavior.
 
   Circuit:
   * Arduino UNO
   * Ethernet shield attached to pins 10, 11, 12, 13
   * Door sensor attached to digitial pin 2
   * Temperature/Humidity (DHT11) sensor attached to digital pin 3
   * Water sensor attached to digital pin 8
 
   Created 22-JAN-2017 by Jon Brule
----------------------------------------------------------------------------- */

#include "DHTSensor.h"
#include "MQTT.h"
#include "WaterSensor.h"

#define DHT_PIN  3
#define DOOR_PIN 2
#define DEVICE_NAME "water_mote_01"
#define MAC_ADDR { 0x90, 0xA2, 0xDA, 0x00, 0x4B, 0x2A }
#define MQTT_SERVER "mqtt.autohome.brule.net"
#define MQTT_USERID "XXX"
#define MQTT_PASSWD "XXX"
#define WATER_PIN 8

byte macAddr[] = MAC_ADDR;
MQTT mqtt(DEVICE_NAME, macAddr, MQTT_SERVER, MQTT_USERID, MQTT_PASSWD);

DHTSensor dhts(DHT_PIN);
int doorState;

WaterSensor waterSensor(WATER_PIN);

boolean startup = true;

void setup() {
  Serial.begin(115200);
  mqtt.setup();
  dhts.setup();
  waterSensor.setup();
  
  pinMode(DOOR_PIN, INPUT_PULLUP);
  doorState = digitalRead(DOOR_PIN);
  attachInterrupt(digitalPinToInterrupt(DOOR_PIN), doorOpen, CHANGE);
  
  delay(1000);
  startup = false;
}

void loop() {
  mqtt.check();
  readSensors();
  delay(10000);
}

void readSensors() {
  float temperature = dhts.temperature();
  float humidity = dhts.humidity();
  boolean waterPresent = waterSensor.measure();

  String json = "{";
  json += "\"temperature\": ";
  json += temperature;
  json += ", \"humidity\": ";
  json += humidity;
  json += ", \"water\": ";
  json += (waterPresent ? "PRESENT" : "NOT PRESENT");
  json += ", \"door\": \"";
  json += (doorState == LOW ? "OPEN" : "CLOSED");
  json += "\"}";
  Serial.print("Reading: "); Serial.println(json);
  mqtt.publish("reading", json.c_str());
}

void doorOpen() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (!startup && (interrupt_time - last_interrupt_time > 200)) {
    doorState = digitalRead(DOOR_PIN);
    String json = "{\"door\": \"";
    json += (doorState == LOW ? "OPEN" : "CLOSED");
    json += "\"}";
    Serial.print("Alert: "); Serial.println(json);
    mqtt.publish("alert", json.c_str());
  }
  last_interrupt_time = interrupt_time;
}
