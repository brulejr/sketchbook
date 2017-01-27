/* -----------------------------------------------------------------------------
   ESP8266 Water Sensor
  
   Monitors water overflow as well as temperature and humidity watching for out-of-ordinary behavior.
 
   Circuit:
   * Arduino UNO
   * Ethernet shield attached to pins 10, 11, 12, 13
   * Door sensor attached to digitial pin 2
   * Temperature/Humidity (DHT11) sensor attached to digital pin 3
   * Water sensor attached to digital pin 8
 
   Created 22-JAN-2017 by Jon Brule
----------------------------------------------------------------------------- */

#include "MQTT.h"

#define DEVICE_NAME     "water-mote-01"
#define RESET_SETTINGS  false
MQTT mqtt(DEVICE_NAME, RESET_SETTINGS);

void setup() {
  Serial.begin(115200);
  mqtt.setup();
}

void loop() {
  mqtt.check();
  delay(10000);
}

