/* -----------------------------------------------------------------------------
   ESP8266 Presence Sensor
  
   Monitors occupancy of a room.
 
   Circuit:
   * ESP8266 (NodeMCU)
   * Door sensor attached to digitial pin 2
 
   Created 28-JAN-2017 by Jon Brule
----------------------------------------------------------------------------- */

#include "MQTT.h"
#include "Sensors.h"

#define RESET_SETTINGS  false
MQTT mqtt(RESET_SETTINGS);

#define ALERT_PERIOD 1000
#define MEASUREMENT_PERIOD 10000

#define DOOR_PIN D2
#define DHT_PIN D5
#define LIGHT_PIN A0
#define MOTION_PIN D3
#define WATER_PIN D4
Sensors sensors(DOOR_PIN, DHT_PIN, LIGHT_PIN, MOTION_PIN, WATER_PIN, &mqtt);

static unsigned long last_alert_time = 0;
static unsigned long last_measurement_time = 0;

void setup() {
  Serial.begin(115200);
  mqtt.setup();
  sensors.setup();
  attachInterrupt(digitalPinToInterrupt(DOOR_PIN), _doorStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(MOTION_PIN), _motionStateChanged, CHANGE);
}

void loop() {
  mqtt.check();
  
  unsigned long current_alert_time = millis();
  if (current_alert_time - last_alert_time > ALERT_PERIOD) {
    last_alert_time = current_alert_time;
    sensors.checkForAlerts();
  }

  unsigned long current_measurement_time = millis();
  if (current_measurement_time - last_measurement_time > MEASUREMENT_PERIOD) {
    last_measurement_time = current_measurement_time;
    sensors.measure();
  }
}

void _doorStateChanged() {
  sensors.handleDoorInterrupt();
}

void _motionStateChanged() {
  sensors.handleMotionInterrupt();
}
