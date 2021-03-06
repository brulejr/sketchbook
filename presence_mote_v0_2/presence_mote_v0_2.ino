/* -----------------------------------------------------------------------------
   ESP8266 Presence Sensor
  
   Monitors occupancy of a room.
 
   Circuit:
   * ESP8266 (NodeMCU)
   * Door sensor attached to digitial pin D2
   * Motion sensor (PIR) attach to digital pin D3
   * Water sensor attached to digital pin D4
   * DHT sensor on digital pin D5
   * Light sensor on analog pin A0
 
   Created 18-FEB-2017 by Jon Brule
----------------------------------------------------------------------------- */

#include "Heartbeat.h"
#include "MQTT.h"
#include "Sensors.h"

#define LED_PIN D7
Heartbeat heartbeat(LED_PIN, 1.0);

#define RESET_WIFI  false
#define RESET_FFS   false
MQTT mqtt(RESET_WIFI, RESET_FFS);

#define ALERT_PERIOD 1000
#define INTERRUPT_TIMER 500
#define MEASUREMENT_PERIOD 10000

#define DOOR_PIN D2
#define DHT_PIN D5
#define LIGHT_PIN A0
#define MOTION_PIN D6
#define WATER_PIN D4
Sensors sensors(DOOR_PIN, DHT_PIN, LIGHT_PIN, MOTION_PIN, WATER_PIN, &mqtt, INTERRUPT_TIMER);

static unsigned long last_alert_time = 0;
static unsigned long last_measurement_time = 0;

void setup() {
  Serial.begin(115200);
  heartbeat.setup();
  mqtt.setup();
  sensors.setup();
  attachInterrupt(digitalPinToInterrupt(DOOR_PIN), _doorStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(MOTION_PIN), _motionStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(WATER_PIN), _waterStateChanged, CHANGE);
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

  heartbeat.beat();
}

void _doorStateChanged() {
  sensors.handleDoorInterrupt();
}

void _motionStateChanged() {
  sensors.handleMotionInterrupt();
}

void _waterStateChanged() {
  sensors.handleWaterInterrupt();
}
