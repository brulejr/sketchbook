/* -----------------------------------------------------------------------------
   Ethernet Water Sensor
  
   Monitors water overflow as well as temperature and humidity watching for out-of-ordinary behavior.
 
   Circuit:
   * Arduino UNO
   * Ethernet shield attached to pins 10, 11, 12, 13
   * Water sensor attached to digitial pin
   * Temperature sensor attached to digital pin 3
 
   Created 21-JAN-2017 by Jon Brule
----------------------------------------------------------------------------- */

#include "DHTSensor.h"
#include "MQTT.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte macAddr[] = { 0x90, 0xA2, 0xDA, 0x00, 0x4B, 0x2A };
char mqttServer[] = "mqtt.dev.brule.net";
MQTT mqtt(macAddr, mqttServer);

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

  delay(1000);
  Serial.print("Temperature: ");
  Serial.print(dhts.temperature());
  Serial.println(" *C");

  Serial.print("Humidity: ");
  Serial.print(dhts.humidity());
  Serial.println("%");
}

