/* -----------------------------------------------------------------------------
   Ethernet Water Sensor
  
   Monitors water overflow as well as temperature and humidity watching for out-of-ordinary behavior.
 
   Circuit:
   * Arduino UNO
   * Ethernet shield attached to pins 10, 11, 12, 13
   * Water sensor attached to digitial pin x
   * Temperature/Humidity (DHT11) sensor attached to digital pin 2
 
   Created 21-JAN-2017 by Jon Brule
----------------------------------------------------------------------------- */

#include "DHTSensor.h"
#include "MQTT.h"

byte macAddr[] = { 0x90, 0xA2, 0xDA, 0x00, 0x4B, 0x2A };
char mqttServer[] = "mqtt.dev.brule.net";
char deviceName[] = "water_mote_01";
MQTT mqtt(macAddr, mqttServer, "water_mote_01");

#define DHTPIN  2
DHTSensor dhts(DHTPIN);
uint32_t delayMS;

void setup() {
  Serial.begin(115200);
  mqtt.setup();
  dhts.setup();
}

void loop() {
  mqtt.check();
  readSensors();
  delay(15000);
}

void readSensors() {
  float temperature = dhts.temperature();
  float humidity = dhts.humidity();

  String json = "{";
  json += "\"temperature\": ";
  json += temperature;
  json += ", \"humidity\": ";
  json += humidity;
  json += "}";
  Serial.print("Reading: "); Serial.println(json);

  mqtt.publish("reading", json.c_str());
}

