/* -----------------------------------------------------------------------------
   ESP8266 Presence Sensor
  
   Monitors occupancy of a room.
 
   Circuit:
   * ESP8266
   * Door sensor attached to digitial pin 2
 
   Created 28-JAN-2017 by Jon Brule
----------------------------------------------------------------------------- */

#include "MQTT.h"
#include "Sensors.h"

#define RESET_SETTINGS  false
MQTT mqtt(RESET_SETTINGS);

#define DOOR_PIN 2
#define LIGHT_PIN A0
#define MOTION_PIN 12
Sensors sensors(DOOR_PIN, LIGHT_PIN, MOTION_PIN, &mqtt);

void setup() {
  Serial.begin(115200);
  mqtt.setup();
  
  attachInterrupt(digitalPinToInterrupt(DOOR_PIN), _doorStateChange, CHANGE);
}

void loop() {
  mqtt.check();
  sensors.check();
  delay(1000);
}

void _doorStateChange() {
  sensors.interrupt();
}
